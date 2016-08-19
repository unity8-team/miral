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

#include "windowmodelnotifier.h"

#include "mirqtconversion.h"
#include <QDebug>

/*
 * WindowModelNotifier - tracks Mir Window Manager operations and duplicates the window stack
 * that Mir has created internally. Any changes to this model are emitted as notify
 * signals to the Qt GUI thread which will effectively duplicate this model again.
 *
 * Use a window ID as a shared identifier between this Mir-side model and the Qt-side model
 */

using namespace qtmir;

WindowModelNotifier::WindowModelNotifier()
{
    qDebug("WindowModelNotifier::WindowModelNotifier");
    qRegisterMetaType<qtmir::WindowInfo>();
}

WindowModelNotifier::~WindowModelNotifier()
{

}

void WindowModelNotifier::addWindow(const miral::WindowInfo &windowInfo)
{
    qDebug("WindowModelNotifier::addWindow");
    auto stackPosition = static_cast<unsigned int>(m_windowStack.count());
    m_windowStack.push_back(windowInfo.window()); // ASSUMPTION: Mir should tell us where in stack

    Q_EMIT windowAdded(windowInfo, stackPosition);
}

void WindowModelNotifier::removeWindow(const miral::WindowInfo &windowInfo)
{
    qDebug("WindowModelNotifier::removeWindow");
    const int pos = m_windowStack.indexOf(windowInfo.window());
    if (pos < 0) {
        qDebug("Unknown window removed");
        return;
    }
    m_windowStack.removeAt(pos);
    auto upos = static_cast<unsigned int>(pos);
    Q_EMIT windowRemoved(upos);
}

void WindowModelNotifier::focusWindow(const miral::WindowInfo &windowInfo, const bool focus)
{
    const int pos = m_windowStack.indexOf(windowInfo.window());
    if (pos < 0) {
        qDebug("Unknown window focused");
        return;
    }
    auto upos = static_cast<unsigned int>(pos);

    if (focus && m_focusedWindowIndex != upos) {
        m_focusedWindowIndex = upos;
        Q_EMIT windowFocused(upos);
    }
}

void WindowModelNotifier::moveWindow(const miral::WindowInfo &windowInfo, mir::geometry::Point topLeft)
{
    const int pos = m_windowStack.indexOf(windowInfo.window());
    if (pos < 0) {
        qDebug("Unknown window moved");
        return;
    }
    auto upos = static_cast<unsigned int>(pos);

    // Note: windowInfo.window() is in the state before the move
    Q_EMIT windowMoved(toQPoint(topLeft), upos);
}

void WindowModelNotifier::resizeWindow(const miral::WindowInfo &windowInfo, mir::geometry::Size newSize)
{
    const int pos = m_windowStack.indexOf(windowInfo.window());
    if (pos < 0) {
        qDebug("Unknown window resized");
        return;
    }
    auto upos = static_cast<unsigned int>(pos);

    // Note: windowInfo.window() is in the state before the resize
    Q_EMIT windowResized(toQSize(newSize), upos);
}

void WindowModelNotifier::raiseWindows(const std::vector<miral::Window> &/*windows*/)
{

}
