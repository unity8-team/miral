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

#include <mir/scene/surface.h>

namespace qtmir {

// miral::WindowInfo contains all the metadata the WindowManager{,Policy} needs. However the
// WindowModel only needs a read-only subset of this data, which is what the struct is for.
struct WindowInfo {
    QSize size;
    QPoint position;
    bool focused;
    const std::shared_ptr<mir::scene::Surface> surface;

    enum class DirtyStates {
        Size = 1,
        Position = 2,
        Focus = 4
    };
};

// We assign each Window with a unique ID that both Mir-side and Qt-side WindowModels can share
struct NumberedWindow {
    unsigned int index;
    WindowInfo windowInfo;
};

struct DirtiedWindow {
    unsigned int index;
    WindowInfo windowInfo;
    WindowInfo::DirtyStates dirtyWindowInfo;
};


class WindowModelInterface : public QObject
{
    Q_OBJECT
public:
    WindowModelInterface() = default;
    virtual ~WindowModelInterface() = default;

Q_SIGNALS:
    void windowAdded(const NumberedWindow);
    void windowRemoved(const unsigned int index);
    void windowChanged(const DirtiedWindow);

private:
    Q_DISABLE_COPY(WindowModelInterface)
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::NumberedWindow)
Q_DECLARE_METATYPE(qtmir::DirtiedWindow)

#endif // WINDOWMODELINTERFACE_H
