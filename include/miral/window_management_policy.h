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

#ifndef MIRAL_WINDOW_MANAGEMENT_POLICY_H
#define MIRAL_WINDOW_MANAGEMENT_POLICY_H

#include <mir/geometry/rectangles.h>
#include <mir_toolkit/event.h>

namespace miral
{
class Window;
class WindowSpecification;
struct ApplicationInfo;
struct WindowInfo;

using namespace mir::geometry;

/// The interface through which the policy invoked.
class WindowManagementPolicy
{
public:
    /// before any related calls begin
    virtual void advise_begin();

    /// after any related calls end
    virtual void advise_end();

/** @name Customize initial window placement
 *  @{ */
    virtual auto place_new_surface(
        ApplicationInfo const& app_info,
        WindowSpecification const& request_parameters) -> WindowSpecification = 0;
/** @} */

/** @name handle events originating from the client
 * The policy is expected to update the model as appropriate
 *  @{ */
    virtual void handle_window_ready(WindowInfo& window_info) = 0;
    virtual void handle_modify_window(WindowInfo& window_info, WindowSpecification const& modifications) = 0;
    virtual void handle_raise_window(WindowInfo& window_info) = 0;
/** @} */

/** @name handle events originating from user
 * The policy is expected to interpret (and optionally consume) the event
 *  @{ */
    virtual bool handle_keyboard_event(MirKeyboardEvent const* event) = 0;
    virtual bool handle_touch_event(MirTouchEvent const* event) = 0;
    virtual bool handle_pointer_event(MirPointerEvent const* event) = 0;
/** @} */

/** @name notification of WM events that the policy may need to track.
 * With the exception of focus_lost/gained these are pre-notifications.
 * \note if the policy updates a Window object directly (as opposed to using tools)
 * no notification is generated.
 *  @{ */
    virtual void advise_new_app(ApplicationInfo& application);
    virtual void advise_delete_app(ApplicationInfo const& application);
    virtual void advise_new_window(WindowInfo& window_info);
    virtual void advise_focus_lost(WindowInfo const& info);
    virtual void advise_focus_gained(WindowInfo const& info);
    virtual void advise_state_change(WindowInfo const& window_info, MirSurfaceState state);
    virtual void advise_move_to(WindowInfo const& window_info, Point top_left);
    virtual void advise_resize(WindowInfo const& window_info, Size const& new_size);
    virtual void advise_delete_window(WindowInfo const& window_info);

    /// \note ordering isn't significant - the existing Z-order will be maintained
    virtual void advise_raise(std::vector<Window> const& windows);
/** @} */

/** @name Changes to the displays
 * \todo this is very course grained and should probably be replaced
 *  @{ */
    virtual void advise_displays_updated(Rectangles const& displays);
/** @} */

    virtual ~WindowManagementPolicy() = default;
    WindowManagementPolicy() = default;
    WindowManagementPolicy(WindowManagementPolicy const&) = delete;
    WindowManagementPolicy& operator=(WindowManagementPolicy const&) = delete;
};

class WindowManagerTools;
}

#endif //MIRAL_WINDOW_MANAGEMENT_POLICY_H
