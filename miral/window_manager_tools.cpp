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

#include "miral/window_manager_tools.h"
#include "window_manager_tools_implementation.h"

miral::WindowManagerTools::WindowManagerTools(WindowManagerToolsImplementation* tools) :
    tools{tools}
{
}

miral::WindowManagerTools::WindowManagerTools(WindowManagerTools const&) = default;
miral::WindowManagerTools& miral::WindowManagerTools::operator=(WindowManagerTools const&) = default;
miral::WindowManagerTools::~WindowManagerTools() = default;

auto miral::WindowManagerTools::count_applications() const -> unsigned int
{ return tools->count_applications(); }

void miral::WindowManagerTools::for_each_application(std::function<void(ApplicationInfo& info)> const& functor)
{ tools->for_each_application(functor); }

auto miral::WindowManagerTools::find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
-> Application
{ return tools->find_application(predicate); }

auto miral::WindowManagerTools::info_for(std::weak_ptr<mir::scene::Session> const& session) const -> ApplicationInfo&
{ return tools->info_for(session); }

auto miral::WindowManagerTools::info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> WindowInfo&
{ return tools->info_for(surface); }

auto miral::WindowManagerTools::info_for(Window const& window) const -> WindowInfo&
{ return tools->info_for(window); }

void miral::WindowManagerTools::kill_active_application(int sig)
{ tools->kill_active_application(sig); }

auto miral::WindowManagerTools::active_window() const -> Window
{ return tools->active_window(); }

auto miral::WindowManagerTools::select_active_window(Window const& hint) -> Window
{ return tools->select_active_window(hint); }

void miral::WindowManagerTools::drag_active_window(mir::geometry::Displacement movement)
{ tools->drag_active_window(movement); }

void miral::WindowManagerTools::focus_next_application()
{ tools->focus_next_application(); }

void miral::WindowManagerTools::focus_next_within_application()
{ tools->focus_next_within_application(); }

auto miral::WindowManagerTools::window_at(mir::geometry::Point cursor) const -> Window
{ return tools->window_at(cursor); }

auto miral::WindowManagerTools::active_display() -> mir::geometry::Rectangle const
{ return tools->active_display(); }

void miral::WindowManagerTools::raise_tree(Window const& root)
{ tools->raise_tree(root); }

void miral::WindowManagerTools::modify_window(WindowInfo& window_info, WindowSpecification const& modifications)
{ tools->modify_window(window_info,modifications); }

void miral::WindowManagerTools::place_and_size(WindowInfo& window_info, Point const& new_pos, Size const& new_size)
{ tools->place_and_size(window_info, new_pos, new_size); }

void miral::WindowManagerTools::set_state(WindowInfo& window_info, MirSurfaceState value)
{ tools->set_state(window_info, value); }

void miral::WindowManagerTools::invoke_under_lock(std::function<void()> const& callback)
{ tools->invoke_under_lock(callback); }
