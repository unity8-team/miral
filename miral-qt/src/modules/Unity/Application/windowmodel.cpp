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

#include "windowmodel.h"

#include "mirsurface.h"

#include <mir/scene/surface.h>

// mirserver
#include "nativeinterface.h"

// Qt
#include <QGuiApplication>
#include <QDebug>

using namespace qtmir;

WindowModel::WindowModel()
{
    auto nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    auto windowModel = static_cast<WindowModelNotifier*>(nativeInterface->nativeResourceForIntegration("WindowModelNotifier"));
    connectToWindowModelNotifier(windowModel);
}

WindowModel::WindowModel(WindowModelNotifier *notifier,
                         WindowControllerInterface *controller)
    : m_windowController(controller)
{
    connectToWindowModelNotifier(notifier);
}

void WindowModel::connectToWindowModelNotifier(WindowModelNotifier *notifier)
{
    connect(notifier, &WindowModelNotifier::windowAdded,        this, &WindowModel::onWindowAdded,        Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowRemoved,      this, &WindowModel::onWindowRemoved,      Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowMoved,        this, &WindowModel::onWindowMoved,        Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowResized,      this, &WindowModel::onWindowResized,      Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowFocusChanged, this, &WindowModel::onWindowFocusChanged, Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowsRaised,      this, &WindowModel::onWindowsRaised,      Qt::QueuedConnection);
}

QHash<int, QByteArray> WindowModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(SurfaceRole, "surface");
    return roleNames;
}

void WindowModel::onWindowAdded(const NewWindow &window)
{
    if (window.windowInfo.type() == mir_surface_type_inputmethod) {
        addInputMethodWindow(window);
        return;
    }

    const int index = m_windowModel.count();
    beginInsertRows(QModelIndex(), index, index);
    m_windowModel.append(new MirSurface(window, m_windowController));
    endInsertRows();
    Q_EMIT countChanged();
}

void WindowModel::onWindowRemoved(const miral::WindowInfo &windowInfo)
{
    if (windowInfo.type() == mir_surface_type_inputmethod) {
        removeInputMethodWindow();
        return;
    }

    const int index = findIndexOf(windowInfo.window());

    beginRemoveRows(QModelIndex(), index, index);
    m_windowModel.takeAt(index);
    endRemoveRows();
    Q_EMIT countChanged();
}

void WindowModel::onWindowMoved(const miral::WindowInfo &windowInfo, const QPoint topLeft)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setPosition(topLeft);
    }
}

void WindowModel::onWindowResized(const miral::WindowInfo &windowInfo, const QSize size)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setSize(size);
    }
}

void WindowModel::onWindowFocusChanged(const miral::WindowInfo &windowInfo, bool focused)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setFocused(focused);
    }
}

void WindowModel::addInputMethodWindow(const NewWindow &windowInfo)
{
    if (m_inputMethodSurface) {
        qDebug("Multiple Input Method Surfaces created, removing the old one!");
        delete m_inputMethodSurface;
    }
    m_inputMethodSurface = new MirSurface(windowInfo, m_windowController);
    Q_EMIT inputMethodSurfaceChanged(m_inputMethodSurface);
}

void WindowModel::removeInputMethodWindow()
{
    if (m_inputMethodSurface) {
        delete m_inputMethodSurface;
        m_inputMethodSurface = nullptr;
        Q_EMIT inputMethodSurfaceChanged(m_inputMethodSurface);
    }
}

void WindowModel::onWindowsRaised(const std::vector<miral::Window> &windows)
{
    const int modelCount = m_windowModel.count();

    QVector<int> indices;
    for (const auto window: windows) {
        int index = findIndexOf(window);
        if (index >= 0) {
            indices.append(index);
        }
    }
    // Assumption: no NO-OPs are in this list - Qt will crash on endMoveRows() if you try NO-OPs!!!
    // A NO-OP is if
    //    1. "indices" is an empty list
    //    2. "indices" of the form (modelCount - 1, modelCount - 2,...) which results in an unchanged list

    // Precompute the list of indices of Windows/Surfaces to raise, including the offsets due to
    // indices which have already been moved.
    QVector<int> moveList;

    for (int i=indices.count()-1; i>=0; i--) {
        const int index = indices[i];
        int moveCount = 0;

        // how many list items under "index" have been moved so far
        for (int j=indices.count()-1; j>i; j--) {
            if (indices[j] < index) {
                moveCount++;
            }
        }

        if (index - moveCount == modelCount - 1) { // is NO-OP, would be moving last element to itself
            continue;
        }

        moveList.prepend(index - moveCount);
    }

    // Perform the moving, trusting the moveList is correct for each iteration.
    QModelIndex parent;
    for (int i=moveList.count()-1; i>=0; i--) {
        const int move = moveList[i];

        beginMoveRows(parent, move, move, parent, modelCount);

        // QVector missing a move method in Qt5.4
        const auto &window = m_windowModel.takeAt(move);
        m_windowModel.push_back(window);

        endMoveRows();
    }
}

int WindowModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_windowModel.count();
}

QVariant WindowModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_windowModel.count())
        return QVariant();

    if (role == SurfaceRole) {
        auto &surface = m_windowModel.at(index.row());
        return QVariant::fromValue(surface);
    } else {
        return QVariant();
    }
}

MirSurface *WindowModel::find(const miral::WindowInfo &needle) const
{
    auto window = needle.window();
    Q_FOREACH(const auto mirSurface, m_windowModel) {
        if (mirSurface->window() == window) {
            return mirSurface;
        }
    }
    return nullptr;
}

int WindowModel::findIndexOf(const miral::Window &needle) const
{
    for (int i=0; i<m_windowModel.count(); i++) {
        if (m_windowModel[i]->window() == needle) {
            return i;
        }
    }
    return -1;
}
