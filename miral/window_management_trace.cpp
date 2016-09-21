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
#include "miral/window_manager_tools.h"
#include "miral/window_specification.h"

miral::WindowManagementTrace::WindowManagementTrace(
    WindowManagerTools const& wrapped,
    WindowManagementPolicyBuilder const& builder) :
    wrapped{wrapped},
    policy(builder(WindowManagerTools{this}))
{
}

auto miral::WindowManagementTrace::count_applications() const -> unsigned int
{
    puts(__func__);
    return wrapped.count_applications();
}

void miral::WindowManagementTrace::for_each_application(std::function<void(miral::ApplicationInfo&)> const& functor)
{
    puts(__func__);
    wrapped.for_each_application(functor);
}

auto miral::WindowManagementTrace::find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
-> Application
{
    puts(__func__);
    return wrapped.find_application(predicate);
}

auto miral::WindowManagementTrace::info_for(std::weak_ptr<mir::scene::Session> const& session) const -> ApplicationInfo&
{
    puts(__func__);
    return wrapped.info_for(session);
}

auto miral::WindowManagementTrace::info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> WindowInfo&
{
    puts(__func__);
    return wrapped.info_for(surface);
}

auto miral::WindowManagementTrace::info_for(Window const& window) const -> WindowInfo&
{
    puts(__func__);
    return wrapped.info_for(window);
}

void miral::WindowManagementTrace::ask_client_to_close(miral::Window const& window)
{
    puts(__func__);
    wrapped.ask_client_to_close(window);
}

auto miral::WindowManagementTrace::active_window() const -> Window
{
    puts(__func__);
    return wrapped.active_window();
}

auto miral::WindowManagementTrace::select_active_window(Window const& hint) -> Window
{
    puts(__func__);
    return wrapped.select_active_window(hint);
}

auto miral::WindowManagementTrace::window_at(mir::geometry::Point cursor) const -> Window
{
    puts(__func__);
    return wrapped.window_at(cursor);
}

auto miral::WindowManagementTrace::active_display() -> mir::geometry::Rectangle const
{
    puts(__func__);
    return wrapped.active_display();
}

auto miral::WindowManagementTrace::info_for_window_id(std::string const& id) const -> WindowInfo&
{
    puts(__func__);
    return wrapped.info_for_window_id(id);
}

auto miral::WindowManagementTrace::id_for_window(Window const& window) const -> std::string
{
    puts(__func__);
    return wrapped.id_for_window(window);
}

void miral::WindowManagementTrace::drag_active_window(mir::geometry::Displacement movement)
{
    puts(__func__);
    wrapped.drag_active_window(movement);
}

void miral::WindowManagementTrace::focus_next_application()
{
    puts(__func__);
    wrapped.focus_next_application();
}

void miral::WindowManagementTrace::focus_next_within_application()
{
    puts(__func__);
    wrapped.focus_next_within_application();
}

void miral::WindowManagementTrace::raise_tree(miral::Window const& root)
{
    puts(__func__);
    wrapped.raise_tree(root);
}

void miral::WindowManagementTrace::modify_window(
    miral::WindowInfo& window_info, miral::WindowSpecification const& modifications)
{
    puts(__func__);
    wrapped.modify_window(window_info, modifications);
}

void miral::WindowManagementTrace::invoke_under_lock(std::function<void()> const& callback)
{
    puts(__func__);
    wrapped.invoke_under_lock(callback);
}

auto miral::WindowManagementTrace::place_new_surface(
    ApplicationInfo const& app_info,
    WindowSpecification const& requested_specification) -> WindowSpecification
{
    puts(__func__);
    return policy->place_new_surface(app_info, requested_specification);
}

void miral::WindowManagementTrace::handle_window_ready(miral::WindowInfo& window_info)
{
    puts(__func__);
    policy->handle_window_ready(window_info);
}

void miral::WindowManagementTrace::handle_modify_window(
    miral::WindowInfo& window_info, miral::WindowSpecification const& modifications)
{
    puts(__func__);
    policy->handle_modify_window(window_info, modifications);
}

void miral::WindowManagementTrace::handle_raise_window(miral::WindowInfo& window_info)
{
    puts(__func__);
    policy->handle_raise_window(window_info);
}

bool miral::WindowManagementTrace::handle_keyboard_event(MirKeyboardEvent const* event)
{
    puts(__func__);
    return policy->handle_keyboard_event(event);
}

bool miral::WindowManagementTrace::handle_touch_event(MirTouchEvent const* event)
{
    puts(__func__);
    return policy->handle_touch_event(event);
}

bool miral::WindowManagementTrace::handle_pointer_event(MirPointerEvent const* event)
{
    puts(__func__);
    return policy->handle_pointer_event(event);
}

void miral::WindowManagementTrace::advise_begin()
{
    puts(__func__);
    policy->advise_begin();
}

void miral::WindowManagementTrace::advise_end()
{
    puts(__func__);
    policy->advise_end();
}

void miral::WindowManagementTrace::advise_new_app(miral::ApplicationInfo& application)
{
    puts(__func__);
    policy->advise_new_app(application);
}

void miral::WindowManagementTrace::advise_delete_app(miral::ApplicationInfo const& application)
{
    puts(__func__);
    policy->advise_delete_app(application);
}

void miral::WindowManagementTrace::advise_new_window(miral::WindowInfo const& window_info)
{
    puts(__func__);
    policy->advise_new_window(window_info);
}

void miral::WindowManagementTrace::advise_focus_lost(miral::WindowInfo const& info)
{
    puts(__func__);
    policy->advise_focus_lost(info);
}

void miral::WindowManagementTrace::advise_focus_gained(miral::WindowInfo const& info)
{
    puts(__func__);
    policy->advise_focus_gained(info);
}

void miral::WindowManagementTrace::advise_state_change(miral::WindowInfo const& window_info, MirSurfaceState state)
{
    puts(__func__);
    policy->advise_state_change(window_info, state);
}

void miral::WindowManagementTrace::advise_move_to(miral::WindowInfo const& window_info, mir::geometry::Point top_left)
{
    puts(__func__);
    policy->advise_move_to(window_info, top_left);
}

void miral::WindowManagementTrace::advise_resize(miral::WindowInfo const& window_info, mir::geometry::Size const& new_size)
{
    puts(__func__);
    policy->advise_resize(window_info, new_size);
}

void miral::WindowManagementTrace::advise_delete_window(miral::WindowInfo const& window_info)
{
    puts(__func__);
    policy->advise_delete_window(window_info);
}

void miral::WindowManagementTrace::advise_raise(std::vector<miral::Window> const& windows)
{
    puts(__func__);
    policy->advise_raise(windows);
}
