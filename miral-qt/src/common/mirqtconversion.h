#ifndef MIRQTCONVERSION_H
#define MIRQTCONVERSION_H

#include <QSize>
#include <QPoint>
#include <QRect>

#include <mir/geometry/size.h>
#include <mir/geometry/point.h>
#include <mir/geometry/rectangle.h>

namespace qtmir {

/*
 * Some handy conversions from Mir types to Qt types and back
 */

inline QSize toQSize(const mir::geometry::Size size)
{
    return QSize(size.width.as_int(), size.height.as_int());
}

inline mir::geometry::Size toMirSize(const QSize size)
{
    namespace mg = mir::geometry;
    return mg::Size{ mg::Width{ size.width()}, mg::Height{ size.height()} };
}

inline QPoint toQPoint(mir::geometry::Point point)
{
    return QPoint(point.x.as_int(), point.y.as_int());
}

inline mir::geometry::Point toMirPoint(const QPoint point)
{
    namespace mg = mir::geometry;
    return mg::Point{ mg::X{ point.x()}, mg::Y{ point.y()} };
}

inline QRect toQRect(const mir::geometry::Rectangle rect)
{
    return QRect(rect.top_left.x.as_int(), rect.top_left.y.as_int(),
                 rect.size.width.as_int(), rect.size.height.as_int());
}

inline mir::geometry::Rectangle toMirRectangle(const QRect rect)
{
    namespace mg = mir::geometry;
    return mg::Rectangle{
        mg::Point{ mg::X{ rect.x()}, mg::Y{ rect.y()} },
        mg::Size{ mg::Width{ rect.width()}, mg::Height{ rect.height()} }
    };
}

} // namespace qtmir

#endif // MIRQTCONVERSION_H
