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

#include "stub_windowmodelnotifier.h"


void StubWindowModelNotifier::emitWindowAdded(const qtmir::WindowInfo windowInfo, const unsigned int index)
{
    Q_EMIT windowAdded(windowInfo, index);
}

void StubWindowModelNotifier::emitWindowRemoved(const unsigned int index)
{
    Q_EMIT windowRemoved(index);
}

void StubWindowModelNotifier::emitWindowMoved(const QPoint topLeft, const unsigned int index)
{
    Q_EMIT windowMoved(topLeft, index);
}

void StubWindowModelNotifier::emitWindowResized(const QSize size, const unsigned int index)
{
    Q_EMIT windowResized(size, index);
}

void StubWindowModelNotifier::emitWindowFocused(const unsigned int index)
{
    Q_EMIT windowFocused(index);
}

void StubWindowModelNotifier::emitWindowInfoChanged(const qtmir::WindowInfo windowInfo, const unsigned int index)
{
    Q_EMIT windowInfoChanged(windowInfo, index);
}
