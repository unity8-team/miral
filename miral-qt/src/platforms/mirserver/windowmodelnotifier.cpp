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
    qRegisterMetaType<qtmir::NewWindowInfo>();
    qRegisterMetaType<qtmir::WindowInfo>();
    qRegisterMetaType<QVector<int>>();
}

WindowModelNotifier::~WindowModelNotifier()
{

}

void WindowModelNotifier::addWindow(const miral::WindowInfo &windowInfo, const std::string &persistentId)
{
    auto stackPosition = m_windowStack.count();
    m_windowStack.push_back(windowInfo.window()); // ASSUMPTION: Mir should tell us where in stack

    NewWindowInfo newWindowInfo{windowInfo, persistentId};
    Q_EMIT windowAdded(newWindowInfo, stackPosition);
}

void WindowModelNotifier::removeWindow(const miral::WindowInfo &windowInfo)
{
    const int pos = m_windowStack.indexOf(windowInfo.window());
    if (pos < 0) {
        qDebug("Unknown window removed");
        return;
    }
    m_windowStack.removeAt(pos);
    Q_EMIT windowRemoved(pos);
}

void WindowModelNotifier::focusWindow(const miral::WindowInfo &windowInfo, const bool focus)
{
    const int pos = m_windowStack.indexOf(windowInfo.window());
    if (pos < 0) {
        qDebug("Unknown window focused");
        return;
    }

    if (focus && m_focusedWindowIndex != pos) {
        m_focusedWindowIndex = pos;
        Q_EMIT windowFocused(pos);
    }
}

void WindowModelNotifier::moveWindow(const miral::WindowInfo &windowInfo, mir::geometry::Point topLeft)
{
    const int pos = m_windowStack.indexOf(windowInfo.window());
    if (pos < 0) {
        qDebug("Unknown window moved");
        return;
    }

    // Note: windowInfo.window() is in the state before the move
    Q_EMIT windowMoved(toQPoint(topLeft), pos);
}

void WindowModelNotifier::resizeWindow(const miral::WindowInfo &windowInfo, mir::geometry::Size newSize)
{
    const int pos = m_windowStack.indexOf(windowInfo.window());
    if (pos < 0) {
        qDebug("Unknown window resized");
        return;
    }

    // Note: windowInfo.window() is in the state before the resize
    Q_EMIT windowResized(toQSize(newSize), pos);
}

void WindowModelNotifier::raiseWindows(const std::vector<miral::Window> &windows)
{
    QVector<int> indices;
    for (auto window: windows) {
        const int pos = m_windowStack.indexOf(window);
        if (pos < 0) {
            qDebug("Unknown window raised");
            continue;
        }
        indices.push_back(pos);
    }

    // Filter some NO-OP (raise list of windows which is already raised and in that order)
    // A NO-OP is if
    //    1. "indices" is an empty list
    //    2. "indices" of the form (modelCount - 1, modelCount - 2,...)
    {
        bool noop = true;
        int counter = m_windowStack.count() - 1;
        Q_FOREACH(int index, indices) {
            if (index != counter) {
                noop = false;
                break;
            }
            counter--;
        }

        if (noop) {
            return;
        }
    }

    for (int i=indices.count()-1; i>=0; i--) {
        // QVector missing a move method in Qt5.4
        auto window = m_windowStack.takeAt(indices[i]);
        m_windowStack.push_back(window);
    }

    Q_EMIT windowsRaised(indices);
}
