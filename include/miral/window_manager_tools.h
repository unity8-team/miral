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
class Surface;
class Session;
struct SurfaceInfo;
struct SessionInfo;

/// The interface through which the policy instructs the controller.
/// These functions assume that the BasicWindowManager data structures can be accessed freely.
/// I.e. should only be invoked by the policy handle_... methods (where any necessary locks are held).
class WindowManagerTools
{
public:
    virtual auto build_surface(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& parameters)
        -> SurfaceInfo& = 0;

    virtual auto count_sessions() const -> unsigned int = 0;

    virtual void for_each_session(std::function<void(SessionInfo& info)> const& functor) = 0;

    virtual auto find_session(std::function<bool(SessionInfo const& info)> const& predicate)
    -> Session = 0;

    virtual auto info_for(std::weak_ptr<mir::scene::Session> const& session) const -> SessionInfo& = 0;

    virtual auto info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> SurfaceInfo& = 0;

    virtual auto info_for(Surface const& surface) const -> SurfaceInfo& = 0;

    virtual auto focused_session() const -> Session = 0;

    virtual auto focused_surface() const -> Surface = 0;

    virtual void focus_next_session() = 0;

    virtual void set_focus_to(Surface const& surface) = 0;

    virtual auto surface_at(mir::geometry::Point cursor) const -> Surface = 0;

    virtual auto active_display() -> mir::geometry::Rectangle const = 0;

    virtual void forget(Surface const& surface) = 0;

    virtual void raise_tree(Surface const& root) = 0;

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
