#ifndef MIRQTCONVERSION_H
#define MIRQTCONVERSION_H

#include <QSize>
#include <QPoint>

#include <mir/geometry/size.h>
#include <mir/geometry/point.h>

namespace qtmir {

/*
 * Some handy conversions from Mir types to Qt types
 */

inline QSize toQSize(mir::geometry::Size size)
{
    return QSize(size.width.as_int(), size.height.as_int());
}

inline QPoint toQPoint(mir::geometry::Point point)
{
    return QPoint(point.x.as_int(), point.y.as_int());
}

} // namespace qtmir

#endif // MIRQTCONVERSION_H
