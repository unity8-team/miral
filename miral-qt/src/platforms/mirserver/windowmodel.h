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

#ifndef WINDOWMODEL_H
#define WINDOWMODEL_H

#include "windowmodelinterface.h"

#include "miral/window_info.h"

#include <QPair>

class WindowModel : public WindowModelInterface
{
    Q_OBJECT
public:
    WindowModel();
    virtual ~WindowModel();

    void addWindow(const miral::WindowInfo &windowInfo);
    void removeWindow(const miral::WindowInfo &windowInfo);
    void focusWindow(const miral::WindowInfo &windowInfo, bool focus);
    void moveWindow(miral::WindowInfo &windowInfo, mir::geometry::Point topLeft);
    void resizeWindow(miral::WindowInfo &windowInfo, mir::geometry::Size newSize);
    void raiseWindows(const std::vector<miral::Window> &windows);  //window?? Not WindowInfo??

private:
    QVector<QPair<miral::WindowInfo, WindowInfo>> m_windowStack;
};

#endif // WINDOWMODEL_H
