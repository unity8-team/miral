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

#include "miral/window_manager_tools.h"
#include "miral/window_specification.h"

#include "mir/scene/surface.h"
#include <QDebug>

WindowManagementPolicy::WindowManagementPolicy(const miral::WindowManagerTools &tools,
                                               qtmir::WindowModel &windowModel,
                                               qtmir::WindowController &windowController,
                                               const QSharedPointer<ScreensModel> screensModel)
    : CanonicalWindowManagerPolicy(tools)
    , m_tools(tools)
    , m_windowModel(windowModel)
    , m_eventFeeder(new QtEventFeeder(screensModel))
{
    windowController.setPolicy(this);
}

/* Following are hooks to allow custom policy be imposed */
miral::WindowSpecification WindowManagementPolicy::place_new_surface(
    const miral::ApplicationInfo &app_info,
    const miral::WindowSpecification &request_parameters)
{
    auto parameters = CanonicalWindowManagerPolicy::place_new_surface(app_info, request_parameters);
    qDebug() << "Place surface" << parameters.top_left().value().x.as_int();
    return parameters;
}

void WindowManagementPolicy::handle_window_ready(miral::WindowInfo &windowInfo)
{
    qDebug("Window ready");
    m_tools.select_active_window(windowInfo.window());
}

void WindowManagementPolicy::handle_modify_window(
    miral::WindowInfo &windowInfo,
    const miral::WindowSpecification &modifications)
{
    qDebug("Window Modified!");
    m_tools.modify_window(windowInfo, modifications);
}

void WindowManagementPolicy::handle_raise_window(miral::WindowInfo &windowInfo)
{
    qDebug("Window Raise");
    m_tools.select_active_window(windowInfo.window());
}

/* Handle input events - here just inject them into Qt event loop for later processing */
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

void WindowManagementPolicy::advise_new_window(miral::WindowInfo &windowInfo)
{
    // TODO: attach surface observer here
    m_windowModel.addWindow(windowInfo);
}

void WindowManagementPolicy::advise_delete_window(const miral::WindowInfo &windowInfo)
{
    m_windowModel.removeWindow(windowInfo);
}

void WindowManagementPolicy::advise_raise(const std::vector<miral::Window> &windows)
{
    m_windowModel.raiseWindows(windows);
}

void WindowManagementPolicy::advise_new_app(miral::ApplicationInfo &/*application*/)
{
    qDebug("New App");
}

void WindowManagementPolicy::advise_delete_app(const miral::ApplicationInfo &/*application*/)
{
    qDebug("Delete App");
}

void WindowManagementPolicy::advise_state_change(const miral::WindowInfo &/*windowInfo*/, MirSurfaceState /*state*/)
{
    qDebug("Window State Change");
}

void WindowManagementPolicy::advise_move_to(const miral::WindowInfo &windowInfo, Point topLeft)
{
    qDebug("Window move");
    m_windowModel.moveWindow(windowInfo, topLeft);
}

void WindowManagementPolicy::advise_resize(const miral::WindowInfo &windowInfo, const Size &newSize)
{
    qDebug("Window Resize");
    m_windowModel.resizeWindow(windowInfo, newSize);
}

void WindowManagementPolicy::advise_focus_lost(const miral::WindowInfo &windowInfo)
{
    m_windowModel.focusWindow(windowInfo, false);
}

void WindowManagementPolicy::advise_focus_gained(const miral::WindowInfo &windowInfo)
{
    m_windowModel.focusWindow(windowInfo, true);
}

void WindowManagementPolicy::advise_begin()
{
    // TODO
}

void WindowManagementPolicy::advise_end()
{
    // TODO
}

/* Following methods all called from the Qt GUI thread to deliver events to clients */
void WindowManagementPolicy::deliver_keyboard_event(const MirKeyboardEvent *event,
                                                    const miral::Window &window)
{
    m_tools.invoke_under_lock([&window, this]() {
        m_tools.select_active_window(window);
    });
    auto e = reinterpret_cast<MirEvent const*>(event); // naughty

    if (auto surface = std::weak_ptr<mir::scene::Surface>(window).lock()) {
        surface->consume(e);
    }}

void WindowManagementPolicy::deliver_touch_event(const MirTouchEvent *event,
                                                 const miral::Window &window)
{
    m_tools.invoke_under_lock([&window, this]() {
        m_tools.select_active_window(window);
    });
    auto e = reinterpret_cast<MirEvent const*>(event); // naughty

    if (auto surface = std::weak_ptr<mir::scene::Surface>(window).lock()) {
        surface->consume(e);
    }
}

void WindowManagementPolicy::deliver_pointer_event(const MirPointerEvent *event,
                                                   const miral::Window &window)
{
    m_tools.invoke_under_lock([&window, this]() {
        m_tools.select_active_window(window);
    });
    auto e = reinterpret_cast<MirEvent const*>(event); // naughty

    if (auto surface = std::weak_ptr<mir::scene::Surface>(window).lock()) {
        surface->consume(e);
    }
}

/* Methods to allow Shell to request changes to the window stack */
void WindowManagementPolicy::focus(const miral::Window &window)
{
    m_tools.select_active_window(window);
}

void WindowManagementPolicy::resize(const miral::Window &window, const Size size)
{
    m_tools.place_and_size(m_tools.info_for(window), window.top_left(), size);
}

void WindowManagementPolicy::move(const miral::Window &window, const Point topLeft)
{
    m_tools.place_and_size(m_tools.info_for(window), topLeft, window.size() );
}
