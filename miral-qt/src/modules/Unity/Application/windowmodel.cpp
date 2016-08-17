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
#include "surfaceobserver.h"

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

    auto windowModel = static_cast<WindowModelNotifierInterface*>(nativeInterface->nativeResourceForIntegration("WindowModel"));
    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    connect(windowModel, &WindowModelNotifierInterface::windowAdded,       this, &WindowModel::onWindowAdded);
    connect(windowModel, &WindowModelNotifierInterface::windowRemoved,     this, &WindowModel::onWindowRemoved);
    connect(windowModel, &WindowModelNotifierInterface::windowMoved,       this, &WindowModel::onWindowMoved);
    connect(windowModel, &WindowModelNotifierInterface::windowResized,     this, &WindowModel::onWindowResized);
    connect(windowModel, &WindowModelNotifierInterface::windowFocused,     this, &WindowModel::onWindowFocused);
    connect(windowModel, &WindowModelNotifierInterface::windowInfoChanged, this, &WindowModel::onWindowInfoChanged);
}

QHash<int, QByteArray> WindowModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(SurfaceRole, "surface");
    return roleNames;
}

void WindowModel::onWindowAdded(const WindowInfo windowInfo, const unsigned int index)
{
    qDebug() << "Window Added!" << index;
    std::shared_ptr<SurfaceObserver> surfaceObserver = std::make_shared<SurfaceObserver>();

    const auto &surface = static_cast<std::shared_ptr<mir::scene::Surface>>(windowInfo.window);
    SurfaceObserver::registerObserverForSurface(surfaceObserver.get(), surface.get());
    surface->add_observer(surfaceObserver);

    auto mirSurface = new MirSurface(windowInfo, m_windowController, surfaceObserver);
    beginInsertRows(QModelIndex(), index, index);
    m_windowModel.insert(index, mirSurface);
    endInsertRows();
    Q_EMIT countChanged();
}

void WindowModel::onWindowRemoved(const unsigned int index)
{
    qDebug() << "Window Removed!" << index;
    beginRemoveRows(QModelIndex(), index, index);
    m_windowModel.remove(index);
    endRemoveRows();
    Q_EMIT countChanged();
}

void WindowModel::onWindowMoved(const QPoint topLeft, const unsigned int index)
{
    auto mirSurface = static_cast<MirSurface *>(m_windowModel.value(index));
    mirSurface->setPosition(topLeft);
}

void WindowModel::onWindowResized(const QSize size, const unsigned int index)
{
    auto mirSurface = static_cast<MirSurface *>(m_windowModel.value(index));
    mirSurface->setSize(size);
}

void WindowModel::onWindowFocused(const unsigned int index)
{
    auto mirSurface = static_cast<MirSurface *>(m_windowModel.value(index));
    if (m_focusedWindow && m_focusedWindow != mirSurface) {
        m_focusedWindow->setFocused(false);
    }
    mirSurface->setFocused(true);
    m_focusedWindow = mirSurface;
}

void WindowModel::onWindowInfoChanged(const WindowInfo windowInfo, const unsigned int pos)
{
    auto mirSurface = static_cast<MirSurface *>(m_windowModel.value(pos));
    mirSurface->updateWindowInfo(windowInfo);

    QModelIndex row = index(pos);
    Q_EMIT dataChanged(row, row, QVector<int>() << SurfaceRole);
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

