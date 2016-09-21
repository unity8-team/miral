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

#include <miral/application_info.h>
#include <miral/window_info.h>

#include <mir/scene/session.h>
#include <mir/scene/surface.h>

#include <sstream>

#define MIR_LOG_COMPONENT "miral::Window Management"
#include <mir/log.h>

namespace 
{
std::string const null_ptr{"(null)"};

inline auto name_of(miral::Window const& window) -> std::string
{
    if (std::shared_ptr<mir::scene::Surface> surface = window)
        return surface->name();
    else
        return null_ptr;
}

inline auto name_of(miral::Application const& application) -> std::string
{
    if (application)
        return application->name();
    else
        return null_ptr;
}
}

miral::WindowManagementTrace::WindowManagementTrace(
    WindowManagerTools const& wrapped,
    WindowManagementPolicyBuilder const& builder) :
    wrapped{wrapped},
    policy(builder(WindowManagerTools{this}))
{
}

auto miral::WindowManagementTrace::count_applications() const -> unsigned int
{
    auto const result = wrapped.count_applications();
    mir::log_info("%s: %d", __func__, result);
    return result;
}

void miral::WindowManagementTrace::for_each_application(std::function<void(miral::ApplicationInfo&)> const& functor)
{
    mir::log_info("%s", __func__);
    wrapped.for_each_application(functor);
}

auto miral::WindowManagementTrace::find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
-> Application
{
    auto result = wrapped.find_application(predicate);
    mir::log_info("%s: %s", __func__, name_of(result).c_str());
    return result;
}

auto miral::WindowManagementTrace::info_for(std::weak_ptr<mir::scene::Session> const& session) const -> ApplicationInfo&
{

    auto& result = wrapped.info_for(session);
    mir::log_info("%s: %s", __func__, result.application()->name().c_str());
    return result;
}

auto miral::WindowManagementTrace::info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> WindowInfo&
{
    auto& result = wrapped.info_for(surface);
    mir::log_info("%s: %s", __func__, result.name().c_str());
    return result;
}

auto miral::WindowManagementTrace::info_for(Window const& window) const -> WindowInfo&
{
    auto& result = wrapped.info_for(window);
    mir::log_info("%s: %s", __func__, result.name().c_str());
    return result;
}

void miral::WindowManagementTrace::ask_client_to_close(miral::Window const& window)
{
    mir::log_info("%s: %s", __func__, name_of(window).c_str());
    wrapped.ask_client_to_close(window);
}

auto miral::WindowManagementTrace::active_window() const -> Window
{
    auto result = wrapped.active_window();
    mir::log_info("%s: %s", __func__, name_of(result).c_str());
    return result;
}

auto miral::WindowManagementTrace::select_active_window(Window const& hint) -> Window
{
    auto result = wrapped.select_active_window(hint);
    mir::log_info("%s hint=%s: %s", __func__, name_of(hint).c_str(), name_of(result).c_str());
    return result;
}

auto miral::WindowManagementTrace::window_at(mir::geometry::Point cursor) const -> Window
{
    auto result = wrapped.window_at(cursor);
    std::stringstream out;
    out << cursor << ": " << name_of(result);
    mir::log_info("%s cursor=%s", __func__, out.str().c_str());
    return result;
}

auto miral::WindowManagementTrace::active_display() -> mir::geometry::Rectangle const
{
    mir::log_info("%s", __func__);
    return wrapped.active_display();
}

auto miral::WindowManagementTrace::info_for_window_id(std::string const& id) const -> WindowInfo&
{
    mir::log_info("%s", __func__);
    return wrapped.info_for_window_id(id);
}

auto miral::WindowManagementTrace::id_for_window(Window const& window) const -> std::string
{
    mir::log_info("%s", __func__);
    return wrapped.id_for_window(window);
}

void miral::WindowManagementTrace::drag_active_window(mir::geometry::Displacement movement)
{
    mir::log_info("%s", __func__);
    wrapped.drag_active_window(movement);
}

void miral::WindowManagementTrace::focus_next_application()
{
    mir::log_info("%s", __func__);
    wrapped.focus_next_application();
}

void miral::WindowManagementTrace::focus_next_within_application()
{
    mir::log_info("%s", __func__);
    wrapped.focus_next_within_application();
}

void miral::WindowManagementTrace::raise_tree(miral::Window const& root)
{
    mir::log_info("%s", __func__);
    wrapped.raise_tree(root);
}

void miral::WindowManagementTrace::modify_window(
    miral::WindowInfo& window_info, miral::WindowSpecification const& modifications)
{
    mir::log_info("%s", __func__);
    wrapped.modify_window(window_info, modifications);
}

void miral::WindowManagementTrace::invoke_under_lock(std::function<void()> const& callback)
{
    mir::log_info("%s", __func__);
    wrapped.invoke_under_lock(callback);
}

auto miral::WindowManagementTrace::place_new_surface(
    ApplicationInfo const& app_info,
    WindowSpecification const& requested_specification) -> WindowSpecification
{
    mir::log_info("%s", __func__);
    return policy->place_new_surface(app_info, requested_specification);
}

void miral::WindowManagementTrace::handle_window_ready(miral::WindowInfo& window_info)
{
    mir::log_info("%s", __func__);
    policy->handle_window_ready(window_info);
}

void miral::WindowManagementTrace::handle_modify_window(
    miral::WindowInfo& window_info, miral::WindowSpecification const& modifications)
{
    mir::log_info("%s", __func__);
    policy->handle_modify_window(window_info, modifications);
}

void miral::WindowManagementTrace::handle_raise_window(miral::WindowInfo& window_info)
{
    mir::log_info("%s", __func__);
    policy->handle_raise_window(window_info);
}

bool miral::WindowManagementTrace::handle_keyboard_event(MirKeyboardEvent const* event)
{
    mir::log_debug("%s", __func__);
    return policy->handle_keyboard_event(event);
}

bool miral::WindowManagementTrace::handle_touch_event(MirTouchEvent const* event)
{
    mir::log_debug("%s", __func__);
    return policy->handle_touch_event(event);
}

bool miral::WindowManagementTrace::handle_pointer_event(MirPointerEvent const* event)
{
    mir::log_debug("%s", __func__);
    return policy->handle_pointer_event(event);
}

void miral::WindowManagementTrace::advise_begin()
{
    mir::log_info("%s", __func__);
    policy->advise_begin();
}

void miral::WindowManagementTrace::advise_end()
{
    mir::log_info("%s", __func__);
    policy->advise_end();
}

void miral::WindowManagementTrace::advise_new_app(miral::ApplicationInfo& application)
{
    mir::log_info("%s", __func__);
    policy->advise_new_app(application);
}

void miral::WindowManagementTrace::advise_delete_app(miral::ApplicationInfo const& application)
{
    mir::log_info("%s", __func__);
    policy->advise_delete_app(application);
}

void miral::WindowManagementTrace::advise_new_window(miral::WindowInfo const& window_info)
{
    mir::log_info("%s", __func__);
    policy->advise_new_window(window_info);
}

void miral::WindowManagementTrace::advise_focus_lost(miral::WindowInfo const& info)
{
    mir::log_info("%s", __func__);
    policy->advise_focus_lost(info);
}

void miral::WindowManagementTrace::advise_focus_gained(miral::WindowInfo const& info)
{
    mir::log_info("%s", __func__);
    policy->advise_focus_gained(info);
}

void miral::WindowManagementTrace::advise_state_change(miral::WindowInfo const& window_info, MirSurfaceState state)
{
    mir::log_info("%s", __func__);
    policy->advise_state_change(window_info, state);
}

void miral::WindowManagementTrace::advise_move_to(miral::WindowInfo const& window_info, mir::geometry::Point top_left)
{
    mir::log_info("%s", __func__);
    policy->advise_move_to(window_info, top_left);
}

void miral::WindowManagementTrace::advise_resize(miral::WindowInfo const& window_info, mir::geometry::Size const& new_size)
{
    mir::log_info("%s", __func__);
    policy->advise_resize(window_info, new_size);
}

void miral::WindowManagementTrace::advise_delete_window(miral::WindowInfo const& window_info)
{
    mir::log_info("%s", __func__);
    policy->advise_delete_window(window_info);
}

void miral::WindowManagementTrace::advise_raise(std::vector<miral::Window> const& windows)
{
    mir::log_info("%s", __func__);
    policy->advise_raise(windows);
}
