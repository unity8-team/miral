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

#include <mir/graphics/display_configuration.h>

#include <memory>

namespace mir
{
namespace scene { class Session; class Surface; struct SurfaceCreationParameters; }
}


namespace miral
{
class Window;
class Application;
struct WindowInfo;
struct ApplicationInfo;

/// The interface through which the policy instructs the controller.
/// These functions assume that the BasicWindowManager data structures can be accessed freely.
/// I.e. should only be invoked by the policy handle_... methods (where any necessary locks are held).
class WindowManagerTools
{
public:
    virtual auto build_window(
        std::shared_ptr<mir::scene::Session> const& session,
        mir::scene::SurfaceCreationParameters const& parameters)
    -> WindowInfo& = 0;
    virtual auto count_applications() const -> unsigned int = 0;
    virtual void for_each_application(std::function<void(ApplicationInfo& info)> const& functor) = 0;
    virtual auto find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
    -> Application = 0;
    virtual auto info_for(std::weak_ptr<mir::scene::Session> const& session) const -> ApplicationInfo& = 0;
    virtual auto info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> WindowInfo& = 0;
    virtual auto info_for(Window const& window) const -> WindowInfo& = 0;
    virtual auto focused_application() const -> Application = 0;
    virtual auto focused_window() const -> Window = 0;
    virtual void focus_next_session() = 0;
    virtual void set_focus_to(Window const& window) = 0;
    virtual auto window_at(mir::geometry::Point cursor) const -> Window = 0;
    virtual auto active_display() -> mir::geometry::Rectangle const = 0;
    virtual void forget(Window const& window) = 0;
    virtual void raise_tree(Window const& root) = 0;
    virtual void size_to_output(mir::geometry::Rectangle& rect) = 0;
    virtual bool place_in_output(mir::graphics::DisplayConfigurationOutputId id,
                                 mir::geometry::Rectangle& rect) = 0;

    virtual ~WindowManagerTools() = default;
    WindowManagerTools() = default;
    WindowManagerTools(WindowManagerTools const&) = delete;
    WindowManagerTools& operator=(WindowManagerTools const&) = delete;
};
}

#endif //MIRAL_WINDOW_MANAGER_TOOLS_H
