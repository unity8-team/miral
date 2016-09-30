/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QTMIR_MIRFOCUSCONTROLLER_H
#define QTMIR_MIRFOCUSCONTROLLER_H

#include <QPointer>

#include "mirsurfaceinterface.h"

namespace qtmir {

class MirSurfaceInterface;

// TODO: Get rid of this class?
class MirFocusController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(unity::shell::application::MirSurfaceInterface* focusedSurface READ focusedSurface
                                                                              WRITE setFocusedSurface
                                                                              NOTIFY focusedSurfaceChanged)
public:
    MirFocusController() : QObject(nullptr) {}
    static MirFocusController* instance();

    void setFocusedSurface(unity::shell::application::MirSurfaceInterface *surface);
    unity::shell::application::MirSurfaceInterface* focusedSurface() const;
    MirSurfaceInterface* previouslyFocusedSurface() { return m_previouslyFocusedSurface.data(); }

Q_SIGNALS:
    void focusedSurfaceChanged();

private:
    static MirFocusController *m_instance;
    QPointer<MirSurfaceInterface> m_previouslyFocusedSurface;
    QPointer<MirSurfaceInterface> m_focusedSurface;
};

} // namespace qtmir

#endif // QTMIR_MIRFOCUSCONTROLLER_H
