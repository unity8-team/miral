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
    : m_focusedWindow(nullptr)
{
    auto nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    auto windowModel = static_cast<WindowModelNotifierInterface*>(nativeInterface->nativeResourceForIntegration("WindowModelNotifier"));
    connectToWindowModelNotifier(windowModel);
}

WindowModel::WindowModel(WindowModelNotifierInterface *notifier,
                         WindowControllerInterface *controller)
    : m_windowController(controller)
{
    connectToWindowModelNotifier(notifier);
}

void WindowModel::connectToWindowModelNotifier(WindowModelNotifierInterface *notifier)
{
    connect(notifier, &WindowModelNotifierInterface::windowAdded,       this, &WindowModel::onWindowAdded,       Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifierInterface::windowRemoved,     this, &WindowModel::onWindowRemoved,     Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifierInterface::windowMoved,       this, &WindowModel::onWindowMoved,       Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifierInterface::windowResized,     this, &WindowModel::onWindowResized,     Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifierInterface::windowFocused,     this, &WindowModel::onWindowFocused,     Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifierInterface::windowInfoChanged, this, &WindowModel::onWindowInfoChanged, Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifierInterface::windowsRaised,     this, &WindowModel::onWindowsRaised,     Qt::QueuedConnection);
}

QHash<int, QByteArray> WindowModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(SurfaceRole, "surface");
    return roleNames;
}

void WindowModel::onWindowAdded(const WindowInfo windowInfo, const int index)
{
    auto mirSurface = new MirSurface(windowInfo, m_windowController);
    beginInsertRows(QModelIndex(), index, index);
    m_windowModel.insert(index, mirSurface);
    endInsertRows();
    Q_EMIT countChanged();
}

void WindowModel::onWindowRemoved(const int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    auto window = m_windowModel.takeAt(index);
    if (window == m_focusedWindow) {
        m_focusedWindow = nullptr;
    }
    endRemoveRows();
    Q_EMIT countChanged();
}

void WindowModel::onWindowMoved(const QPoint topLeft, const int index)
{
    auto mirSurface = static_cast<MirSurface *>(m_windowModel.value(index));
    mirSurface->setPosition(topLeft);
}

void WindowModel::onWindowResized(const QSize size, const int index)
{
    auto mirSurface = static_cast<MirSurface *>(m_windowModel.value(index));
    mirSurface->setSize(size);
}

void WindowModel::onWindowFocused(const int index)
{
    auto mirSurface = static_cast<MirSurface *>(m_windowModel.value(index));
    if (m_focusedWindow && m_focusedWindow != mirSurface) {
        m_focusedWindow->setFocused(false);
    }
    mirSurface->setFocused(true);
    m_focusedWindow = mirSurface;
}

void WindowModel::onWindowInfoChanged(const WindowInfo windowInfo, const int pos)
{
    auto mirSurface = static_cast<MirSurface *>(m_windowModel.value(pos));
    mirSurface->updateWindowInfo(windowInfo);

    QModelIndex row = index(pos);
    Q_EMIT dataChanged(row, row, QVector<int>() << SurfaceRole);
}

void WindowModel::onWindowsRaised(QVector<int> indices)
{
    const int modelCount = m_windowModel.count();

    // Filter some NO-OPs - Qt will crash on endMoveRows() if you try NO-OPs!!!
    // A NO-OP is if
    //    1. "indices" is an empty list
    //    2. "indices" of the form (modelCount - 1, modelCount - 2,...)
    {
        bool noop = true;
        int counter = modelCount - 1;
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

    // Ok, not a NO-OP. Precompute the list of indices of Windows/Surfaces to
    // raise, including the offsets due to indices which have already been moved.
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
        const auto window = m_windowModel.takeAt(move);
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
        MirSurfaceInterface *surface = m_windowModel.at(index.row());
        return QVariant::fromValue(static_cast<MirSurfaceInterface*>(surface));
    } else {
        return QVariant();
    }
}

