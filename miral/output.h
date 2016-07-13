/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIRAL_OUTPUT_H
#define MIRAL_OUTPUT_H

#include <mir_toolkit/common.h>

#include <mir/geometry/rectangle.h>
#include <mir/int_wrapper.h>

#include <memory>

namespace mir
{
namespace graphics
{
namespace detail { struct GraphicsConfOutputIdTag; }
typedef IntWrapper<detail::GraphicsConfOutputIdTag> DisplayConfigurationOutputId;
class DisplayConfigurationOutput;
}
}

namespace miral
{
using namespace mir::geometry;

class Output
{
public:

    struct PhysicalSize { float width; float height; };
//    struct Mode { Size size; double vrefresh_hz; };

    enum class Type
    {
        unknown,
        vga,
        dvii,
        dvid,
        dvia,
        composite,
        svideo,
        lvds,
        component,
        ninepindin,
        displayport,
        hdmia,
        hdmib,
        tv,
        edp
    };

    Output(const mir::graphics::DisplayConfigurationOutput &output);

    /// The output's id.
    auto id() const -> mir::graphics::DisplayConfigurationOutputId;

    /// The type of the output.
    auto type() const -> Type;

    /// The physical size of the output.
    auto physical_size() const -> PhysicalSize;

    /// Whether the output is connected.
    auto connected() const -> bool;

    /// Whether the output is used in the configuration.
    auto used() const -> bool;

    /// The current output pixel format.
    auto pixel_format() const -> MirPixelFormat;
    
    /// refresh_rate in Hz
    auto refresh_rate() const -> double;

    /// Current power mode **/
    auto power_mode() const -> MirPowerMode;

    auto orientaton() const -> MirOrientation;

    /// Requested scale factor for this output, for HiDPI support
    auto scale() const -> float;

    /// Form factor of this output; phone display, tablet, monitor, TV, projector...
    auto form_factor() const -> MirFormFactor;

    /// The logical rectangle occupied by the output, based on its position,
    /// current mode and orientation (rotation)
    auto extents() const -> Rectangle;

    auto valid() const -> bool;

private:
    std::shared_ptr<mir::graphics::DisplayConfigurationOutput> self;
};
}


#endif //MIRAL_OUTPUT_H
