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

    connect(windowModel, &WindowModelInterface::windowAdded, this, &WindowModel::windowAdded);
    connect(windowModel, &WindowModelInterface::windowRemoved, this, &WindowModel::windowRemoved);
    connect(windowModel, &WindowModelInterface::windowChanged, this, &WindowModel::windowChanged);
}

QHash<int, QByteArray> WindowModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(SurfaceRole, "surface");
    return roleNames;
}

void WindowModel::windowAdded(const NumberedWindow window)
{
    std::shared_ptr<SurfaceObserver> surfaceObserver = std::make_shared<SurfaceObserver>();
    const auto &surface = window.windowInfo.surface;
    SurfaceObserver::registerObserverForSurface(surfaceObserver.get(), surface.get());
    surface->add_observer(surfaceObserver);

    auto mirSurface = new MirSurface(surface, nullptr, nullptr, surfaceObserver, CreationHints());
    beginInsertRows(QModelIndex(), window.index, window.index);
    m_windowModel.insert(window.index, mirSurface);
    endInsertRows();
    Q_EMIT countChanged();
}

void WindowModel::windowRemoved(const unsigned int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_windowModel.remove(index);
    endRemoveRows();
    Q_EMIT countChanged();
}

void WindowModel::windowChanged(const DirtiedWindow)
{

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

