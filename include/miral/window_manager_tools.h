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

/// The interface through which the policy instructs the controller.
class WindowManagerTools
{
public:
/** @name Update Model
 *  These functions assume that the BasicWindowManager data structures can be accessed freely.
 *  I.e. they should only be used by a thread that has called the WindowManagementPolicy methods
 *  (where any necessary locks are held) or via a invoke_under_lock() callback.
 *  @{ */
    virtual auto build_window(Application const& application, WindowSpecification const& parameters)
    -> WindowInfo& = 0;
    virtual auto count_applications() const -> unsigned int = 0;
    virtual void for_each_application(std::function<void(ApplicationInfo& info)> const& functor) = 0;
    virtual auto find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
    -> Application = 0;
    virtual auto info_for(std::weak_ptr<mir::scene::Session> const& session) const -> ApplicationInfo& = 0;
    virtual auto info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> WindowInfo& = 0;
    virtual auto info_for(Window const& window) const -> WindowInfo& = 0;
    virtual void kill_active_application(int sig) = 0;
    virtual auto active_window() const -> Window = 0;
    virtual auto select_active_window(Window const& hint) -> Window = 0;
    virtual void focus_next_application() = 0;
    virtual auto window_at(mir::geometry::Point cursor) const -> Window = 0;
    virtual auto active_display() -> mir::geometry::Rectangle const = 0;
    virtual void destroy(Window& window) = 0;
    virtual void raise_tree(Window const& root) = 0;
    virtual void move_tree(miral::WindowInfo& root, mir::geometry::Displacement movement) = 0;
    virtual void size_to_output(mir::geometry::Rectangle& rect) = 0;
    virtual bool place_in_output(int id, mir::geometry::Rectangle& rect) = 0;
/** @} */

/** @name Multi-thread support
 *  Allows threads that don't hold a lock on the model to acquire one and call the "Update Model"
 *  member functions.
 *  This should NOT be used by a thread that has called the WindowManagementPolicy methods (and
 *  already holds the lock).
 *  @{ */
    virtual void invoke_under_lock(std::function<void()> const& callback) = 0;
/** @} */

    virtual ~WindowManagerTools() = default;
    WindowManagerTools() = default;
    WindowManagerTools(WindowManagerTools const&) = delete;
    WindowManagerTools& operator=(WindowManagerTools const&) = delete;
};
}

#endif //MIRAL_WINDOW_MANAGER_TOOLS_H
