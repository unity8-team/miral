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

#ifndef WINDOWMODELINTERFACE_H
#define WINDOWMODELINTERFACE_H

#include <QObject>
#include <QSize>
#include <QPoint>
#include <QVector>


struct WindowInfo {
    QSize size;
    QPoint position;
    bool focused;

    enum class DirtyStates {
      Size = 1,
      Position = 2,
      Focused = 4
    };
};

struct NumberedWindowList {
    const unsigned int index;
    const WindowInfo windowInfo;
};

struct ChangedWindowList {
    const unsigned int index;
    const WindowInfo windowInfo;
    const WindowInfo::DirtyStates dirtyWindowInfo;
};

class WindowModelInterface : public QObject
{
    Q_OBJECT
public:
    WindowModelInterface() = default;
    virtual ~WindowModelInterface() = default;

Q_SIGNALS:
    void windowsAdded(const NumberedWindowList);
    void windowsRemoved(const NumberedWindowList);
    void windowsChanged(const ChangedWindowList);

private:
    Q_DISABLE_COPY(WindowModelInterface)
};

#endif // WINDOWMODELINTERFACE_H
