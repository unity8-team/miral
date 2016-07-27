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

miral::WindowManagerToolsIndirect::WindowManagerToolsIndirect(WindowManagerTools* tools) :
    tools{tools}
{
}

miral::WindowManagerToolsIndirect::WindowManagerToolsIndirect(WindowManagerToolsIndirect const&) = default;
miral::WindowManagerToolsIndirect& miral::WindowManagerToolsIndirect::operator=(WindowManagerToolsIndirect const&) = default;
miral::WindowManagerToolsIndirect::~WindowManagerToolsIndirect() = default;

auto miral::WindowManagerToolsIndirect::count_applications() const -> unsigned int
{ return tools->count_applications(); }

void miral::WindowManagerToolsIndirect::for_each_application(std::function<void(ApplicationInfo& info)> const& functor)
{ tools->for_each_application(functor); }

auto miral::WindowManagerToolsIndirect::find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
-> Application
{ return tools->find_application(predicate); }

auto miral::WindowManagerToolsIndirect::info_for(std::weak_ptr<mir::scene::Session> const& session) const -> ApplicationInfo&
{ return tools->info_for(session); }

auto miral::WindowManagerToolsIndirect::info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> WindowInfo&
{ return tools->info_for(surface); }

auto miral::WindowManagerToolsIndirect::info_for(Window const& window) const -> WindowInfo&
{ return tools->info_for(window); }

void miral::WindowManagerToolsIndirect::kill_active_application(int sig)
{ tools->kill_active_application(sig); }

auto miral::WindowManagerToolsIndirect::active_window() const -> Window
{ return tools->active_window(); }

auto miral::WindowManagerToolsIndirect::select_active_window(Window const& hint) -> Window
{ return tools->select_active_window(hint); }

void miral::WindowManagerToolsIndirect::drag_active_window(mir::geometry::Displacement movement)
{ tools->drag_active_window(movement); }

void miral::WindowManagerToolsIndirect::focus_next_application()
{ tools->focus_next_application(); }

void miral::WindowManagerToolsIndirect::focus_next_within_application()
{ tools->focus_next_within_application(); }

auto miral::WindowManagerToolsIndirect::window_at(mir::geometry::Point cursor) const -> Window
{ return tools->window_at(cursor); }

auto miral::WindowManagerToolsIndirect::active_display() -> mir::geometry::Rectangle const
{ return tools->active_display(); }

void miral::WindowManagerToolsIndirect::raise_tree(Window const& root)
{ tools->raise_tree(root); }

void miral::WindowManagerToolsIndirect::modify_window(WindowInfo& window_info, WindowSpecification const& modifications)
{ tools->modify_window(window_info,modifications); }

void miral::WindowManagerToolsIndirect::place_and_size(WindowInfo& window_info, Point const& new_pos, Size const& new_size)
{ tools->place_and_size(window_info, new_pos, new_size); }

void miral::WindowManagerToolsIndirect::set_state(WindowInfo& window_info, MirSurfaceState value)
{ tools->set_state(window_info, value); }
