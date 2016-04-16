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

#ifndef MIRAL_WINDOW_SPECIFICATION_H
#define MIRAL_WINDOW_SPECIFICATION_H

#include <mir_toolkit/common.h>

#include <mir/int_wrapper.h>
#include <mir/optional_value.h>

#include <mir/geometry/displacement.h>
#include <mir/geometry/rectangles.h>

#include <memory>

namespace mir
{
namespace scene { class Surface; class SurfaceCreationParameters; }
namespace shell { class SurfaceSpecification; }
}

namespace miral
{
using namespace mir::geometry;
namespace detail { struct SessionsBufferStreamIdTag; }
typedef mir::IntWrapper<detail::SessionsBufferStreamIdTag> BufferStreamId;

class WindowSpecification
{
public:
    enum class BufferUsage
    {
        undefined,
        /** rendering using GL */
            hardware,
        /** rendering using direct pixel access */
            software
    };

    enum class InputReceptionMode
    {
        normal,
        receives_all_input
    };

    struct AspectRatio { unsigned width; unsigned height; };

    struct StreamSpecification
    {
        BufferStreamId stream_id;
        Displacement displacement;
    };

    WindowSpecification();
//    WindowSpecification(WindowSpecification const& that);
//    auto operator=(WindowSpecification const& that) => WindowSpecification&;

    WindowSpecification(mir::shell::SurfaceSpecification const& spec);
    auto operator=(mir::shell::SurfaceSpecification const& spec) -> WindowSpecification&;
    void update(mir::shell::SurfaceSpecification& spec) const;

    WindowSpecification(mir::scene::SurfaceCreationParameters const& params);
    auto operator=(mir::scene::SurfaceCreationParameters const& params) -> WindowSpecification&;
    void update(mir::scene::SurfaceCreationParameters& params) const;

    ~WindowSpecification();

    auto top_left() const -> mir::optional_value<Point>;
    auto size() const -> mir::optional_value<Size>;
    auto pixel_format() const -> mir::optional_value<MirPixelFormat>;
    auto buffer_usage() const -> mir::optional_value<BufferUsage>;
    auto name() const -> mir::optional_value<std::string>;
    auto output_id() const -> mir::optional_value<int>;
    auto type() const -> mir::optional_value<MirSurfaceType>;
    auto state() const -> mir::optional_value<MirSurfaceState>;
    auto preferred_orientation() const -> mir::optional_value<MirOrientationMode>;
    auto aux_rect() const -> mir::optional_value<Rectangle>;
    auto edge_attachment() const -> mir::optional_value<MirEdgeAttachment>;
    auto min_width() const -> mir::optional_value<Width>;
    auto min_height() const -> mir::optional_value<Height>;
    auto max_width() const -> mir::optional_value<Width>;
    auto max_height() const -> mir::optional_value<Height>;
    auto width_inc() const -> mir::optional_value<DeltaX>;
    auto height_inc() const -> mir::optional_value<DeltaY>;
    auto min_aspect() const -> mir::optional_value<AspectRatio>;
    auto max_aspect() const -> mir::optional_value<AspectRatio>;
    auto streams() const -> mir::optional_value<std::vector<StreamSpecification>>;
    auto parent() const -> mir::optional_value<std::weak_ptr<mir::scene::Surface>>;
    auto input_shape() const -> mir::optional_value<std::vector<Rectangle>>;
    auto input_mode() const -> mir::optional_value<InputReceptionMode>;
    auto shell_chrome() const -> mir::optional_value<MirShellChrome>;

    void top_left(mir::optional_value<Point> const& value);
    void size(mir::optional_value<Size> const& value);
    void pixel_format(mir::optional_value<MirPixelFormat> const& value);
    void buffer_usage(mir::optional_value<BufferUsage> const& value);
    void name(mir::optional_value<std::string> const& value);
    void output_id(mir::optional_value<int> const& value);
    void type(mir::optional_value<MirSurfaceType> const& value);
    void state(mir::optional_value<MirSurfaceState> const& value);
    void preferred_orientation(mir::optional_value<MirOrientationMode> const& value);
    void aux_rect(mir::optional_value<Rectangle> const& value);
    void edge_attachment(mir::optional_value<MirEdgeAttachment> const& value);
    void min_width(mir::optional_value<Width> const& value);
    void min_height(mir::optional_value<Height> const& value);
    void max_width(mir::optional_value<Width> const& value);
    void max_height(mir::optional_value<Height> const& value);
    void width_inc(mir::optional_value<DeltaX> const& value);
    void height_inc(mir::optional_value<DeltaY> const& value);
    void min_aspect(mir::optional_value<AspectRatio> const& value);
    void max_aspect(mir::optional_value<AspectRatio> const& value);
    void streams(mir::optional_value<std::vector<StreamSpecification>> const& value);
    void parent(mir::optional_value<std::weak_ptr<mir::scene::Surface>> const& value);
    void input_shape(mir::optional_value<std::vector<Rectangle>> const& value);
    void input_mode(mir::optional_value<InputReceptionMode> const& value);
    void shell_chrome(mir::optional_value<MirShellChrome> const& value);

private:
    struct Self;
    std::unique_ptr<Self> self;
};
}

#endif //MIRAL_WINDOW_SPECIFICATION_H
