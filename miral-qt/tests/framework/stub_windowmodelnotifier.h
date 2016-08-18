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

#ifndef STUBWINDOWMODELNOTIFIER_H
#define STUBWINDOWMODELNOTIFIER_H

#include <QObject>

#include "windowmodelnotifierinterface.h"

class StubWindowModelNotifier : public qtmir::WindowModelNotifierInterface
{
public:
    StubWindowModelNotifier() = default;

    void emitWindowAdded(const qtmir::WindowInfo windowInfo, const unsigned int index);
    void emitWindowRemoved(const unsigned int index);
    void emitWindowMoved(const QPoint topLeft, const unsigned int index);
    void emitWindowResized(const QSize size, const unsigned int index);
    void emitWindowFocused(const unsigned int index);
    void emitWindowInfoChanged(const qtmir::WindowInfo windowInfo, const unsigned int index);
};

#endif // STUBWINDOWMODELNOTIFIER_H
