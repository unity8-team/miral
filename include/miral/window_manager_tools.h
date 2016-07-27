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

#ifndef MIRAL_WINDOW_MANAGER_TOOLS_H
#define MIRAL_WINDOW_MANAGER_TOOLS_H

#include "miral/application.h"
#include "window_info.h"

#include <mir/geometry/displacement.h>

#include <functional>
#include <memory>

namespace mir
{
namespace scene { class Surface; }
}

namespace miral
{
class Window;
struct WindowInfo;
struct ApplicationInfo;
class WindowSpecification;

class WindowManagerTools;

class WindowManagerToolsIndirect
{
public:
    WindowManagerToolsIndirect(WindowManagerTools* tools);
    WindowManagerToolsIndirect(WindowManagerToolsIndirect const&);
    WindowManagerToolsIndirect& operator=(WindowManagerToolsIndirect const&);
    ~WindowManagerToolsIndirect();

/** @name Update Model
 *  These functions assume that the BasicWindowManager data structures can be accessed freely.
 *  I.e. they should only be used by a thread that has called the WindowManagementPolicy methods
 *  (where any necessary locks are held) or via a invoke_under_lock() callback.
 *  @{ */
    auto count_applications() const -> unsigned int;
    void for_each_application(std::function<void(ApplicationInfo& info)> const& functor);
    auto find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
    -> Application;
    auto info_for(std::weak_ptr<mir::scene::Session> const& session) const -> ApplicationInfo&;
    auto info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> WindowInfo&;
    auto info_for(Window const& window) const -> WindowInfo&;
    void kill_active_application(int sig);
    auto active_window() const -> Window;
    auto select_active_window(Window const& hint) -> Window;
    void drag_active_window(mir::geometry::Displacement movement);
    void focus_next_application();
    void focus_next_within_application();
    auto window_at(mir::geometry::Point cursor) const -> Window;
    auto active_display() -> mir::geometry::Rectangle const;
    void raise_tree(Window const& root);
    void modify_window(WindowInfo& window_info, WindowSpecification const& modifications);
    void place_and_size(WindowInfo& window_info, Point const& new_pos, Size const& new_size);
    void set_state(WindowInfo& window_info, MirSurfaceState value);
/** @} */

/** @name Multi-thread support
 *  Allows threads that don't hold a lock on the model to acquire one and call the "Update Model"
 *  member functions.
 *  This should NOT be used by a thread that has called the WindowManagementPolicy methods (and
 *  already holds the lock).
 *  @{ */
    void invoke_under_lock(std::function<void()> const& callback);
/** @} */

private:
    WindowManagerTools* tools;
};
}

#endif //MIRAL_WINDOW_MANAGER_TOOLS_H
