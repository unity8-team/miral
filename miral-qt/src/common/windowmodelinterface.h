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
// miral::WindowInfo has a copy constructor, but implicitly shares a Self object between each
// copy. If a miral::WindowInfo copy is sent over signal/slot connection across thread boundaries,
// it could be changed in a Mir thread before the slot processes it.
//
// This is undesirable as we need to update the GUI thread model in a consistent way.
//
// Instead we duplicate the miral::WindowInfo data, in a way that can be sent over signal/slot
// connections safely.

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
