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

#ifndef WINDOWMANAGEMENTPOLICY_H
#define WINDOWMANAGEMENTPOLICY_H

#include "miral/window_management_policy.h"

#include "qteventfeeder.h"

#include <QObject>
#include <QScopedPointer>
#include <QSize>

using namespace mir::geometry;

class ScreensModel;

class WindowManagementPolicy : public QObject, public miral::WindowManagementPolicy
{
public:
    WindowManagementPolicy(const miral::WindowManagerTools *tools,
                           const QSharedPointer<ScreensModel> &screensModel);


    auto place_new_surface(const miral::ApplicationInfo &app_info,
                           const miral::WindowSpecification &request_parameters)
        -> miral::WindowSpecification override;

    void handle_window_ready(miral::WindowInfo &windowInfo) override;
    void handle_modify_window(
            miral::WindowInfo &windowInfo,
            const miral::WindowSpecification &modifications) override;
    void handle_raise_window(miral::WindowInfo &windowInfo) override;

    bool handle_keyboard_event(const MirKeyboardEvent *event) override;
    bool handle_touch_event(const MirTouchEvent *event) override;
    bool handle_pointer_event(const MirPointerEvent *event) override;

    void advise_new_window(miral::WindowInfo &windowInfo) override;
    void advise_focus_lost(const miral::WindowInfo &info) override;
    void advise_focus_gained(const miral::WindowInfo &info) override;
    void advise_state_change(const miral::WindowInfo &info, MirSurfaceState state) override;
    void advise_resize(const miral::WindowInfo &info, const Size &newSize) override;
    void advise_delete_window(const miral::WindowInfo &windowInfo) override;

    void handle_app_info_updated(const Rectangles &displays) override;
    void handle_displays_updated(const Rectangles &displays) override;

Q_SIGNALS:
//    void sessionCreatedSurface(mir::scene::Session const*,
//                               std::shared_ptr<mir::scene::Surface> const&,
//                               std::shared_ptr<SurfaceObserver> const&,
//                               qtmir::CreationHints);
//    void sessionDestroyingSurface(mir::scene::Session const*, std::shared_ptr<mir::scene::Surface> const&);

//    // requires Qt::BlockingQueuedConnection!!
//    void sessionAboutToCreateSurface(const miral::ApplicationInfo &app_info,
//                                     const miral::WindowSpecification &request_parameters);


private:
    const miral::WindowManagerTools *m_tools;
    const QScopedPointer<QtEventFeeder> m_eventFeeder;
};

#endif // WINDOWMANAGEMENTPOLICY_H
