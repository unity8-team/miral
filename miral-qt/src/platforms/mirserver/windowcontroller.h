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

#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include "windowcontrollerinterface.h"

class WindowManagementPolicy;

namespace qtmir {

class WindowController : public WindowControllerInterface
{
public:
    WindowController();
    virtual ~WindowController() = default;

    void focus (const miral::Window &window) override;
    void resize(const miral::Window &window, const QSize &size) override;
    void move  (const miral::Window &window, const QPoint &topLeft) override;

    void setActiveFocus(const miral::Window &window, const bool activeFocus) override;
    void setState(const miral::Window &window, const MirSurfaceState state) override;

    void deliverKeyboardEvent(const miral::Window &window, const MirKeyboardEvent *event) override;
    void deliverTouchEvent   (const miral::Window &window, const MirTouchEvent *event) override;
    void deliverPointerEvent (const miral::Window &window, const MirPointerEvent *event) override;

    void setPolicy(WindowManagementPolicy *policy);

protected:
    WindowManagementPolicy *m_policy;
};

} // namespace qtmir

#endif // WINDOWCONTROLLER_H
