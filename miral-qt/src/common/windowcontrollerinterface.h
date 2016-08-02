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

#ifndef WINDOWCONTROLLERINTERFACE_H
#define WINDOWCONTROLLERINTERFACE_H

#include "miral/window.h"
#include "mir/geometry/point.h"
#include "mir/geometry/size.h"
#include "mir_toolkit/event.h"

class MirSurface;

namespace qtmir {

class WindowControllerInterface {
public:
    WindowControllerInterface() = default;
    virtual ~WindowControllerInterface() = default;

    virtual void focus (const miral::Window &window) = 0;
    virtual void resize(const miral::Window &window, const mir::geometry::Size size) = 0;
    virtual void move  (const miral::Window &window, const mir::geometry::Point topLeft) = 0;

    virtual void deliver_keyboard_event(const MirKeyboardEvent *event, const miral::Window &window) = 0;
    virtual void deliver_touch_event   (const MirTouchEvent *event,    const miral::Window &window) = 0;
    virtual void deliver_pointer_event (const MirPointerEvent *event,  const miral::Window &window) = 0;
};

} // namespace qtmir

#endif // WINDOWCONTROLLERINTERFACE_H
