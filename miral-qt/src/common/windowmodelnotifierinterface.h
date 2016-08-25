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

#ifndef WINDOWMODELNOTIFIERINTERFACE_H
#define WINDOWMODELNOTIFIERINTERFACE_H

#include <QObject>
#include <QSize>
#include <QPoint>
#include <QVector>

#include "miral/window_info.h"

namespace qtmir {

// miral::WindowInfo missing a default constructor, needed by MOC. Need to wrap it instead
class WindowInfo {
public:
    WindowInfo() = default;
    WindowInfo(const miral::WindowInfo &windowInfo)
      : window(windowInfo.window())
      , name(windowInfo.name())
      , type(windowInfo.type())
      , state(windowInfo.state())
      , restoreRect(windowInfo.restore_rect())
      , parent(windowInfo.parent())
      , children(windowInfo.children())
      , minWidth(windowInfo.min_width())
      , minHeight(windowInfo.min_height())
      , maxWidth(windowInfo.max_width())
      , maxHeight(windowInfo.max_height())
    {}

    miral::Window window;
    mir::optional_value<std::string> name;
    MirSurfaceType type;
    MirSurfaceState state;
    mir::geometry::Rectangle restoreRect;
    miral::Window parent;
    std::vector<miral::Window> children;
    mir::geometry::Width minWidth;
    mir::geometry::Height minHeight;
    mir::geometry::Width maxWidth;
    mir::geometry::Height maxHeight;
};


class WindowModelNotifierInterface : public QObject
{
    Q_OBJECT
public:
    WindowModelNotifierInterface() = default;
    virtual ~WindowModelNotifierInterface() = default;

Q_SIGNALS:
    void windowAdded(const qtmir::WindowInfo, const int index);
    void windowRemoved(const int index);
    void windowMoved(const QPoint topLeft, const int index);
    void windowResized(const QSize size, const int index);
    void windowFocused(const int index);
    void windowInfoChanged(const qtmir::WindowInfo, const int index);
    void windowsRaised(const QVector<int> indices);

private:
    Q_DISABLE_COPY(WindowModelNotifierInterface)
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::WindowInfo)

#endif // WINDOWMODELNOTIFIERINTERFACE_H
