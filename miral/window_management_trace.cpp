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

#include "window_management_trace.h"

#include "miral/window.h"
#include "miral/window_specification.h"

auto miral::WindowManagementTrace::count_applications() const -> unsigned int
{
    return 0;
}

void miral::WindowManagementTrace::for_each_application(std::function<void(miral::ApplicationInfo&)> const& /*functor*/)
{

}

auto miral::WindowManagementTrace::find_application(std::function<bool(ApplicationInfo const& info)> const& /*predicate*/)
-> Application
{
    return {};
}

auto miral::WindowManagementTrace::info_for(std::weak_ptr<mir::scene::Session> const& /*session*/) const -> ApplicationInfo&
{
    throw "TODO";
}

auto miral::WindowManagementTrace::info_for(std::weak_ptr<mir::scene::Surface> const& /*surface*/) const -> WindowInfo&
{
    throw "TODO";
}

auto miral::WindowManagementTrace::info_for(Window const& /*window*/) const -> WindowInfo&
{
    throw "TODO";
}

void miral::WindowManagementTrace::ask_client_to_close(miral::Window const& /*window*/)
{

}

auto miral::WindowManagementTrace::active_window() const -> Window
{
    return {};
}

auto miral::WindowManagementTrace::select_active_window(Window const& /*hint*/) -> Window
{
    return {};
}

auto miral::WindowManagementTrace::window_at(mir::geometry::Point /*cursor*/) const -> Window
{
    return {};
}

auto miral::WindowManagementTrace::active_display() -> mir::geometry::Rectangle const
{
    return {};
}

auto miral::WindowManagementTrace::info_for_window_id(std::string const& /*id*/) const -> WindowInfo&
{
    throw "TODO";
}

auto miral::WindowManagementTrace::id_for_window(Window const& /*window*/) const -> std::string
{
    return {};
}

void miral::WindowManagementTrace::drag_active_window(mir::geometry::Displacement /*movement*/)
{

}

void miral::WindowManagementTrace::focus_next_application()
{

}

void miral::WindowManagementTrace::focus_next_within_application()
{

}

void miral::WindowManagementTrace::raise_tree(miral::Window const& /*root*/)
{

}

void miral::WindowManagementTrace::modify_window(
    miral::WindowInfo& /*window_info*/, miral::WindowSpecification const& /*modifications*/)
{

}

void miral::WindowManagementTrace::invoke_under_lock(std::function<void()> const& /*callback*/)
{

}

auto miral::WindowManagementTrace::place_new_surface(
    ApplicationInfo const& /*app_info*/,
    WindowSpecification const& requested_specification) -> WindowSpecification
{
    return requested_specification;
}

void miral::WindowManagementTrace::handle_window_ready(miral::WindowInfo& /*window_info*/)
{

}

void miral::WindowManagementTrace::handle_modify_window(
    miral::WindowInfo& /*window_info*/, miral::WindowSpecification const& /*modifications*/)
{

}

void miral::WindowManagementTrace::handle_raise_window(miral::WindowInfo& /*window_info*/)
{

}

bool miral::WindowManagementTrace::handle_keyboard_event(MirKeyboardEvent const* /*event*/)
{
    return false;
}

bool miral::WindowManagementTrace::handle_touch_event(MirTouchEvent const* /*event*/)
{
    return false;
}

bool miral::WindowManagementTrace::handle_pointer_event(MirPointerEvent const* /*event*/)
{
    return false;
}

static miral::WindowManagementTrace test_instance;