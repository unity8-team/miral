/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

#include "surfaceobserver.h"

#include <QImage>
#include <QCursor>
#include <QMetaObject>
#include <QMutableMapIterator>
#include <QMutexLocker>
#include <QPixmap>

#include <miral/window_specification.h>
#include <mir/geometry/size.h>


namespace {

QRect calculateBoundingRect(const std::vector<mir::geometry::Rectangle> &rectVector)
{
    QRect boundingRect;
    for (auto mirRect : rectVector) {
        boundingRect |= QRect(mirRect.top_left.x.as_int(),
                mirRect.top_left.y.as_int(),
                mirRect.size.width.as_int(),
                mirRect.size.height.as_int());
    }
    return boundingRect;
}

} // anonymous namespace

QHash<const mir::scene::Surface*, SurfaceObserver*> SurfaceObserver::m_surfaceToObserverMap;
QMutex SurfaceObserver::mutex;


SurfaceObserver::~SurfaceObserver()
{
    QMutexLocker locker(&mutex);
    QMutableHashIterator<const mir::scene::Surface*, SurfaceObserver*> i(m_surfaceToObserverMap);
    while (i.hasNext()) {
        i.next();
        if (i.value() == this) {
            i.remove();
            return;
        }
    }
}

void SurfaceObserver::notifySurfaceModifications(const miral::WindowSpecification &modifications)
{
    if (modifications.min_width().is_set()) {
        Q_EMIT minimumWidthChanged(modifications.min_width().value().as_int());
    }
    if (modifications.min_height().is_set()) {
        Q_EMIT minimumHeightChanged(modifications.min_height().value().as_int());
    }
    if (modifications.max_width().is_set()) {
        Q_EMIT maximumWidthChanged(modifications.max_width().value().as_int());
    }
    if (modifications.max_height().is_set()) {
        Q_EMIT maximumHeightChanged(modifications.max_height().value().as_int());
    }
    if (modifications.width_inc().is_set()) {
        Q_EMIT widthIncrementChanged(modifications.width_inc().value().as_int());
    }
    if (modifications.height_inc().is_set()) {
        Q_EMIT heightIncrementChanged(modifications.height_inc().value().as_int());
    }
    if (modifications.shell_chrome().is_set()) {
        Q_EMIT shellChromeChanged(modifications.shell_chrome().value());
    }
    if (modifications.input_shape().is_set()) {
        QRect qRect = calculateBoundingRect(modifications.input_shape().value());
        Q_EMIT inputBoundsChanged(qRect);
    }
    if (modifications.confine_pointer().is_set()) {
        Q_EMIT confinesMousePointerChanged(modifications.confine_pointer().value() == mir_pointer_confined_to_surface);
    }
}

SurfaceObserver *SurfaceObserver::observerForSurface(const mir::scene::Surface *surface)
{
    if (m_surfaceToObserverMap.contains(surface)) {
        return m_surfaceToObserverMap.value(surface);
    } else {
        return nullptr;
    }
}

void SurfaceObserver::registerObserverForSurface(SurfaceObserver *observer, const mir::scene::Surface *surface)
{
    QMutexLocker locker(&mutex);
    m_surfaceToObserverMap[surface] = observer;
}
