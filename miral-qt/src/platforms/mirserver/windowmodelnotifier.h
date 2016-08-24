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

#include "windowmodelnotifierinterface.h"

#include "miral/window_info.h"

#include <QPair>

namespace qtmir {

class WindowModelNotifier : public WindowModelNotifierInterface
{
    Q_OBJECT
public:
    WindowModelNotifier();
    virtual ~WindowModelNotifier();

    void addWindow(const miral::WindowInfo &windowInfo);
    void removeWindow(const miral::WindowInfo &windowInfo);

    void moveWindow(const miral::WindowInfo &windowInfo, mir::geometry::Point topLeft);
    void resizeWindow(const miral::WindowInfo &windowInfo, mir::geometry::Size newSize);

    void focusWindow(const miral::WindowInfo &windowInfo, const bool focus);
    void raiseWindows(const std::vector<miral::Window> &windows);  //window?? Not WindowInfo??

private:
    QVector<miral::Window> m_windowStack;
    int m_focusedWindowIndex;
};

} // namespace qtmir

#endif // WINDOWMODELNOTIFIER_H
