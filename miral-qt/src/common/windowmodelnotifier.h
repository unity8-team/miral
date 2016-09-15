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

#ifndef WINDOWMODELNOTIFIER_H
#define WINDOWMODELNOTIFIER_H

#include <QObject>
#include <QPoint>
#include <QSize>

#include "miral/window_info.h"

namespace qtmir {

class NewWindow {
public:
    NewWindow() = default;
    NewWindow(const miral::WindowInfo &windowInfo, const std::string &persistentId = "")
      : windowInfo(windowInfo)
      , persistentId(persistentId)
      , surface(windowInfo.window())
    {}

    miral::WindowInfo windowInfo;
    std::string persistentId;

    // hold copy of Surface shared pointer, as miral::Window has just a weak pointer to the Surface
    // but MirSurface needs to share ownership of the Surface with Mir
    std::shared_ptr<mir::scene::Surface> surface;
};


class WindowModelNotifier : public QObject
{
    Q_OBJECT
public:
    WindowModelNotifier() = default;

Q_SIGNALS: // **Must used Queued Connection or else events will be out of order**
    void windowAdded(const qtmir::NewWindow &window);
    void windowRemoved(const miral::WindowInfo &window);
    void windowMoved(const miral::WindowInfo &window, const QPoint topLeft);
    void windowResized(const miral::WindowInfo &window, const QSize size);
    void windowStateChanged(const miral::WindowInfo &window, MirSurfaceState state);
    void windowFocusChanged(const miral::WindowInfo &window, bool focused);
    void windowsRaised(const std::vector<miral::Window> &windows); // results in deep copy when passed over Queued connection:(

private:
    Q_DISABLE_COPY(WindowModelNotifier)
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::NewWindow)
Q_DECLARE_METATYPE(miral::WindowInfo)
Q_DECLARE_METATYPE(std::vector<miral::Window>)
Q_DECLARE_METATYPE(MirSurfaceState)

#endif // WINDOWMODELNOTIFIER_H
