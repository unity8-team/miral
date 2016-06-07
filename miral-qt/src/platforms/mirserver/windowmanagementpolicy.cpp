/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "windowmanagementpolicy.h"

#include "screensmodel.h"

#include "miral/window_specification.h"

WindowManagementPolicy::WindowManagementPolicy(const miral::WindowManagerTools *tools,
                                               const QSharedPointer<ScreensModel> screensModel)
    : m_tools(tools)
    , m_eventFeeder(new QtEventFeeder(screensModel))
{
}


void WindowManagementPolicy::handle_app_info_updated(const Rectangles &/*displays*/)
{
    Q_UNUSED(m_tools); // REMOVEME once m_tools is used (keep clang happy)
}

void WindowManagementPolicy::handle_displays_updated(const Rectangles &/*displays*/)
{

}

miral::WindowSpecification WindowManagementPolicy::place_new_surface(
    const miral::ApplicationInfo &/*app_info*/,
    const miral::WindowSpecification &request_parameters)
{
    auto parameters = request_parameters;
    return parameters;
}

void WindowManagementPolicy::advise_new_window(miral::WindowInfo &/*windowInfo*/)
{

}

void WindowManagementPolicy::handle_window_ready(miral::WindowInfo &/*windowInfo*/)
{

}

void WindowManagementPolicy::handle_modify_window(
    miral::WindowInfo &/*windowInfo*/,
    const miral::WindowSpecification &/*modifications*/)
{

}

void WindowManagementPolicy::advise_delete_window(const miral::WindowInfo &/*windowInfo*/)
{

}

bool WindowManagementPolicy::handle_keyboard_event(const MirKeyboardEvent *event)
{
    m_eventFeeder->dispatchKey(event);
    return true;
}

bool WindowManagementPolicy::handle_touch_event(const MirTouchEvent *event)
{
    m_eventFeeder->dispatchTouch(event);
    return true;
}

bool WindowManagementPolicy::handle_pointer_event(const MirPointerEvent *event)
{
    m_eventFeeder->dispatchPointer(event);
    return true;
}

void WindowManagementPolicy::advise_new_app(miral::ApplicationInfo &/*application*/)
{

}

void WindowManagementPolicy::advise_delete_app(const miral::ApplicationInfo &/*application*/)
{

}

void WindowManagementPolicy::advise_state_change(const miral::WindowInfo &/*windowInfo*/, MirSurfaceState /*state*/)
{

}

void WindowManagementPolicy::advise_resize(const miral::WindowInfo &/*info*/, const Size &/*newSize*/)
{

}

void WindowManagementPolicy::handle_raise_window(miral::WindowInfo &/*windowInfo*/)
{

}

void WindowManagementPolicy::advise_focus_lost(const miral::WindowInfo &/*info*/)
{

}

void WindowManagementPolicy::advise_focus_gained(const miral::WindowInfo &/*info*/)
{

}
