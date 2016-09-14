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

#include <mir/scene/surface.h>
#include <QDebug>

WindowManagementPolicy::WindowManagementPolicy(const miral::WindowManagerTools &tools,
                                               qtmir::WindowModelNotifier &windowModel,
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
    qDebug("Window Ready");
    CanonicalWindowManagerPolicy::handle_window_ready(windowInfo);
}

void WindowManagementPolicy::handle_modify_window(
    miral::WindowInfo &windowInfo,
    const miral::WindowSpecification &modifications)
{
    qDebug("Window Modified!");
    CanonicalWindowManagerPolicy::handle_modify_window(windowInfo, modifications);
}

void WindowManagementPolicy::handle_raise_window(miral::WindowInfo &windowInfo)
{
    qDebug("Window Raise");
    CanonicalWindowManagerPolicy::handle_raise_window(windowInfo);
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

void WindowManagementPolicy::advise_new_window(const miral::WindowInfo &windowInfo)
{
    // TODO: attach surface observer here
    std::string persistentId = m_tools.id_for_window(windowInfo.window());

    m_windowModel.addWindow(windowInfo, persistentId);
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
    qDebug("Window Moved to (%d, %d)", topLeft.x.as_int(), topLeft.y.as_int());
    m_windowModel.moveWindow(windowInfo, topLeft);
}

void WindowManagementPolicy::advise_resize(const miral::WindowInfo &windowInfo, const Size &newSize)
{
    qDebug("Window Resized to %dx%d", newSize.width.as_int(), newSize.height.as_int());
    m_windowModel.resizeWindow(windowInfo, newSize);
}

void WindowManagementPolicy::advise_focus_lost(const miral::WindowInfo &windowInfo)
{
    m_windowModel.focusWindow(windowInfo, false);
}

void WindowManagementPolicy::advise_focus_gained(const miral::WindowInfo &windowInfo)
{
    // update Qt model ASAP, before applying Mir policy
    m_windowModel.focusWindow(windowInfo, true);

    CanonicalWindowManagerPolicy::advise_focus_gained(windowInfo);
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
    }
}

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
    // Prevent mouse hover events causing window focus to change
    if (mir_pointer_event_action(event) == mir_pointer_action_button_down) {
        m_tools.invoke_under_lock([&window, this]() {
            m_tools.select_active_window(window);
        });
    }
    auto e = reinterpret_cast<MirEvent const*>(event); // naughty

    if (auto surface = std::weak_ptr<mir::scene::Surface>(window).lock()) {
        surface->consume(e);
    }
}

/* Methods to allow Shell to request changes to the window stack. Called from the Qt GUI thread */
void WindowManagementPolicy::focus(const miral::Window &window)
{
    m_tools.invoke_under_lock([&window, this]() {
        m_tools.select_active_window(window);
    });
}

void WindowManagementPolicy::resize(const miral::Window &window, const Size size)
{
    miral::WindowSpecification modifications;
    modifications.size() = size;
    m_tools.invoke_under_lock([&window, &modifications, this]() {
        try {
            m_tools.modify_window(m_tools.info_for(window), modifications);
        } catch (const std::out_of_range&) {
            // usually shell trying to operate on a window which already closed, just ignore
        }
    });
}

void WindowManagementPolicy::move(const miral::Window &window, const Point topLeft)
{
    miral::WindowSpecification modifications;
    modifications.top_left() = topLeft;
    m_tools.invoke_under_lock([&window, &modifications, this]() {
        try {
            m_tools.modify_window(m_tools.info_for(window), modifications);
        } catch (const std::out_of_range&) {
            // usually shell trying to operate on a window which already closed, just ignore
        }
    });
}

void WindowManagementPolicy::ask_client_to_close(const miral::Window &window)
{
    m_tools.invoke_under_lock([&window, this]() {
        m_tools.ask_client_to_close(window);
    });
}
