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

class WindowManagementPolicy
{
public:
    virtual void handle_app_info_updated(mir::geometry::Rectangles const& displays) = 0;
    virtual void handle_displays_updated(mir::geometry::Rectangles const& displays) = 0;
    virtual auto handle_place_new_surface(
        ApplicationInfo const& app_info,
        WindowSpecification const& request_parameters) -> WindowSpecification = 0;
    virtual void handle_new_window(WindowInfo& window_info) = 0;
    virtual void handle_window_ready(WindowInfo& window_info) = 0;
    virtual void handle_modify_window(WindowInfo& window_info, WindowSpecification const& modifications) = 0;
    virtual void handle_delete_window(WindowInfo& window_info) = 0;

    // TOOD set_state is really a special (and legacy) case of handle_modify_window() and should be removed
    virtual auto handle_set_state(WindowInfo& window_info, MirSurfaceState value) -> MirSurfaceState = 0;

    virtual bool handle_keyboard_event(MirKeyboardEvent const* event) = 0;
    virtual bool handle_touch_event(MirTouchEvent const* event) = 0;
    virtual bool handle_pointer_event(MirPointerEvent const* event) = 0;
    virtual void handle_raise_window(WindowInfo& window_info) = 0;

    virtual auto select_active_window(Window const& hint) -> Window = 0;

    virtual ~WindowManagementPolicy() = default;
    WindowManagementPolicy() = default;
    WindowManagementPolicy(WindowManagementPolicy const&) = delete;
    WindowManagementPolicy& operator=(WindowManagementPolicy const&) = delete;
};

class WindowManagerTools;
}

#endif //MIRAL_WINDOW_MANAGEMENT_POLICY_H
