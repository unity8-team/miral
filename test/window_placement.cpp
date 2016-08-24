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

#include <mir/shell/display_layout.h>
#include <mir/shell/persistent_surface_store.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

auto window_management_policy_builder = [](miral::WindowManagerTools const& /*tools*/)
    -> std::unique_ptr<miral::WindowManagementPolicy> { return {}; };

struct WindowPlacement : testing::Test
{
    StubFocusController focus_controller;
    std::shared_ptr<StubDisplayLayout> const display_layout{std::make_shared<StubDisplayLayout>()};
    std::shared_ptr<StubPersistentSurfaceStore> const persistent_surface_store{std::make_shared<StubPersistentSurfaceStore>()};

    miral::BasicWindowManager basicwindowmanager{&focus_controller, display_layout, persistent_surface_store, window_management_policy_builder};
};

}