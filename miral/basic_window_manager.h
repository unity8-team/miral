/*
 * Copyright © 2015-2016 Canonical Ltd.
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
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIR_ABSTRACTION_BASIC_WINDOW_MANAGER_H_
#define MIR_ABSTRACTION_BASIC_WINDOW_MANAGER_H_

#include "miral/window_management_policy.h"
#include "miral/window_manager_tools.h"
#include "miral/surface_info.h"
#include "miral/session.h"
#include "miral/session_info.h"

#include "mir/geometry/rectangles.h"
#include "mir/shell/abstract_shell.h"
#include "mir/shell/window_manager.h"

#include <map>
#include <mutex>

namespace mir
{
namespace shell { class DisplayLayout; }
}

/// This is based on mir/examples, but intended to move to miral after building the necessary abstractions
namespace miral
{
using mir::shell::SurfaceSet;

/// A policy based window manager.
/// This takes care of the management of any meta implementation held for the sessions and surfaces.
class BasicWindowManager : public virtual mir::shell::WindowManager,
    protected WindowManagerTools
{
public:
    BasicWindowManager(
        mir::shell::FocusController* focus_controller,
        std::shared_ptr<mir::shell::DisplayLayout> const& display_layout,
        std::unique_ptr<WindowManagementPolicy> (*build)(WindowManagerTools* tools));

    auto build_surface(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& parameters)
    -> SurfaceInfo& override;

    void add_session(std::shared_ptr<mir::scene::Session> const& session) override;

    void remove_session(std::shared_ptr<mir::scene::Session> const& session) override;

    auto add_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        mir::scene::SurfaceCreationParameters const& params,
        std::function<mir::frontend::SurfaceId(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params)> const& build)
    -> mir::frontend::SurfaceId override;

    void modify_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::scene::Surface> const& surface,
        mir::shell::SurfaceSpecification const& modifications) override;

    void remove_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        std::weak_ptr<mir::scene::Surface> const& surface) override;

    void forget(Window const& surface) override;

    void add_display(mir::geometry::Rectangle const& area) override;

    void remove_display(mir::geometry::Rectangle const& area) override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    void handle_raise_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::scene::Surface> const& surface,
        uint64_t timestamp) override;

    int set_surface_attribute(
        std::shared_ptr<mir::scene::Session> const& /*session*/,
        std::shared_ptr<mir::scene::Surface> const& surface,
        MirSurfaceAttrib attrib,
        int value) override;

    auto count_sessions() const -> unsigned int override;

    void for_each_session(std::function<void(SessionInfo& info)> const& functor) override;

    auto find_session(std::function<bool(SessionInfo const& info)> const& predicate)
    -> Session override;

    auto info_for(std::weak_ptr<mir::scene::Session> const& session) const -> SessionInfo& override;

    auto info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> SurfaceInfo& override;

    auto info_for(Window const& surface) const -> SurfaceInfo& override;

    auto focused_session() const -> Session override;

    auto focused_surface() const -> Window override;

    void focus_next_session() override;

    void set_focus_to(Window const& surface) override;

    auto surface_at(mir::geometry::Point cursor) const -> Window override;

    auto active_display() -> mir::geometry::Rectangle const override;

    void raise_tree(Window const& root) override;

    void size_to_output(mir::geometry::Rectangle& rect) override;

    bool place_in_output(mir::graphics::DisplayConfigurationOutputId id, mir::geometry::Rectangle& rect) override;

private:
    using SurfaceInfoMap = std::map<std::weak_ptr<mir::scene::Surface>, SurfaceInfo, std::owner_less<std::weak_ptr<mir::scene::Surface>>>;
    using SessionInfoMap = std::map<std::weak_ptr<mir::scene::Session>, SessionInfo, std::owner_less<std::weak_ptr<mir::scene::Session>>>;

    mir::shell::FocusController* const focus_controller;
    std::shared_ptr<mir::shell::DisplayLayout> const display_layout;
    std::unique_ptr<WindowManagementPolicy> const policy;

    std::mutex mutex;
    SessionInfoMap session_info;
    SurfaceInfoMap surface_info;
    mir::geometry::Rectangles displays;
    mir::geometry::Point cursor;
    uint64_t last_input_event_timestamp{0};

    // Cache the builder functor for the convenience of policies - this should become unnecessary
    std::function<Window(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params)> surface_builder;

    void update_event_timestamp(MirKeyboardEvent const* kev);
    void update_event_timestamp(MirPointerEvent const* pev);
    void update_event_timestamp(MirTouchEvent const* tev);
};
}

#endif /* MIR_ABSTRACTION_BASIC_WINDOW_MANAGER_H_ */
