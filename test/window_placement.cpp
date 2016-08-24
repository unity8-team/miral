/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "../miral/basic_window_manager.h"

#include <miral/canonical_window_manager.h>

#include <mir/frontend/surface_id.h>
#include <mir/scene/surface_creation_parameters.h>
#include <mir/shell/display_layout.h>
#include <mir/shell/persistent_surface_store.h>

#include <mir/test/doubles/stub_session.h>
#include <mir/test/doubles/stub_surface.h>
#include <mir/test/fake_shared.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>

using namespace miral;
using namespace testing;
namespace mt = mir::test;

namespace
{
struct StubFocusController : mir::shell::FocusController
{
    void focus_next_session() override
    {
    }

    auto focused_session() const -> std::shared_ptr<mir::scene::Session> override
    {
        return {};
    }

    void set_focus_to(
        std::shared_ptr<mir::scene::Session> const& /*focus_session*/,
        std::shared_ptr<mir::scene::Surface> const& /*focus_surface*/) override
    {
    }

    auto focused_surface() const -> std::shared_ptr<mir::scene::Surface> override
    {
        return {};
    }

    void raise(mir::shell::SurfaceSet const& /*surfaces*/) override
    {
    }

    virtual auto surface_at(mir::geometry::Point /*cursor*/) const -> std::shared_ptr<mir::scene::Surface> override
    {
        return {};
    }

};

struct StubDisplayLayout : mir::shell::DisplayLayout
{
    void clip_to_output(mir::geometry::Rectangle& /*rect*/) override
    {
    }

    void size_to_output(mir::geometry::Rectangle& /*rect*/) override
    {
    }

    bool place_in_output(mir::graphics::DisplayConfigurationOutputId /*id*/, mir::geometry::Rectangle& /*rect*/) override
    {
        return false;
    }
};

struct StubPersistentSurfaceStore : mir::shell::PersistentSurfaceStore
{
    Id id_for_surface(std::shared_ptr<mir::scene::Surface> const& /*surface*/) override
    {
        return {};
    }

    auto surface_for_id(Id const& /*id*/) const -> std::shared_ptr<mir::scene::Surface> override
    {
        return {};
    }
};

struct StubStubSession : mir::test::doubles::StubSession
{
    mir::frontend::SurfaceId create_surface(
        mir::scene::SurfaceCreationParameters const& /*params*/,
        std::shared_ptr<mir::frontend::EventSink> const& /*sink*/) override
    {
        auto id = mir::frontend::SurfaceId{next_surface_id.fetch_add(1)};
        auto surface = std::make_shared<mir::test::doubles::StubSurface>();
        surfaces[id] = surface;
        return id;
    }

    std::shared_ptr<mir::scene::Surface> surface(mir::frontend::SurfaceId surface) const override
    {
        return surfaces.at(surface);
    }

private:
    std::atomic<int> next_surface_id;
    std::map<mir::frontend::SurfaceId, std::shared_ptr<mir::scene::Surface>> surfaces;
};

struct MockWindowManagerPolicy : CanonicalWindowManagerPolicy
{
    using CanonicalWindowManagerPolicy::CanonicalWindowManagerPolicy;

    bool handle_touch_event(MirTouchEvent const* /*event*/) override { return false; }
    bool handle_pointer_event(MirPointerEvent const* /*event*/) override { return false; }
    bool handle_keyboard_event(MirKeyboardEvent const* /*event*/) override { return false; }

    MOCK_METHOD1(advise_new_window, void (WindowInfo const& window_info));
};

X const display_left{0};
Y const display_top{0};
Width  const display_width{640};
Height const display_height{480};

Rectangle const display_area{{display_left, display_top}, {display_width, display_height}};

struct WindowPlacement : testing::Test
{
    StubFocusController focus_controller;
    StubDisplayLayout display_layout;
    StubPersistentSurfaceStore persistent_surface_store;
    std::shared_ptr<StubStubSession> session{std::make_shared<StubStubSession>()};

    MockWindowManagerPolicy* window_manager_policy;
    BasicWindowManager basic_window_manager{
        &focus_controller,
        mt::fake_shared(display_layout),
        mt::fake_shared(persistent_surface_store),
        [this](WindowManagerTools const& tools) -> std::unique_ptr<WindowManagementPolicy> 
            {
                auto policy = std::make_unique<MockWindowManagerPolicy>(tools);
                window_manager_policy = policy.get();
                return policy;
            }
        };

    Window parent;

    void SetUp() override
    {
        basic_window_manager.add_display(display_area);

        mir::scene::SurfaceCreationParameters creation_parameters;
        basic_window_manager.add_session(session);

        EXPECT_CALL(*window_manager_policy, advise_new_window(_))
            .WillOnce(Invoke([this](WindowInfo const& window_info){ parent = window_info.window(); }));

        basic_window_manager.add_surface(session, creation_parameters, &WindowPlacement::create_surface);
    }

    static auto create_surface(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params)
    -> mir::frontend::SurfaceId
    {
        // This type is Mir-internal, I hope we don't need to create it here
        std::shared_ptr<mir::frontend::EventSink> const sink;

        return session->create_surface(params, sink);
    }
};
}

TEST_F(WindowPlacement, fixture_doesnt_crash)
{
    ASSERT_THAT(parent, Ne(Window{}));
}
