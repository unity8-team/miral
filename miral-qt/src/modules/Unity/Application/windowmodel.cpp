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
{
    auto nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    auto windowModel = static_cast<WindowModelInterface*>(nativeInterface->nativeResourceForIntegration("WindowModel"));
    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    connect(windowModel, &WindowModelInterface::windowAdded,   this, &WindowModel::onWindowAdded);
    connect(windowModel, &WindowModelInterface::windowRemoved, this, &WindowModel::onWindowRemoved);
    connect(windowModel, &WindowModelInterface::windowChanged, this, &WindowModel::onWindowChanged);
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

void WindowModel::onWindowChanged(const WindowInfo /*windowInfo*/, const unsigned int pos)
{
    qDebug() << "Window Change!" << pos;
//    auto mirSurface = m_windowModel.value(pos);

//    switch(window.dirtyWindowInfo) {
//    case WindowInfo::DirtyStates::Size: {
//        qDebug() << "size";
//        // Do nothing yet, it gets new size from swapped buffer for now
//    }
//    case WindowInfo::DirtyStates::Position:
//        qDebug() << "position";
//        mirSurface->setPosition(window.windowInfo.position);
//    case WindowInfo::DirtyStates::Focus:
//        qDebug() << "focus";
//        mirSurface->setFocused(window.windowInfo.focused);
//    }

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

