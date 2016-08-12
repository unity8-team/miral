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

#include "miral/stream_specification.h"

#include <mir_toolkit/common.h>
#include <mir/optional_value.h>
#include <mir/geometry/rectangles.h>

#include <memory>

namespace mir
{
namespace scene { class Surface; struct SurfaceCreationParameters; }
namespace shell { struct SurfaceSpecification; }
}

namespace miral
{
using namespace mir::geometry;

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

    WindowSpecification();
    WindowSpecification(WindowSpecification const& that);
    auto operator=(WindowSpecification const& that) -> WindowSpecification&;

    WindowSpecification(mir::shell::SurfaceSpecification const& spec);
    WindowSpecification(mir::scene::SurfaceCreationParameters const& params);
    void update(mir::scene::SurfaceCreationParameters& params) const;

    ~WindowSpecification();

    auto top_left() const -> mir::optional_value<Point> const&;
    auto size() const -> mir::optional_value<Size> const&;
    auto pixel_format() const -> mir::optional_value<MirPixelFormat> const&;
    auto buffer_usage() const -> mir::optional_value<BufferUsage> const&;
    auto name() const -> mir::optional_value<std::string> const&;
    auto output_id() const -> mir::optional_value<int> const&;
    auto type() const -> mir::optional_value<MirSurfaceType> const&;
    auto state() const -> mir::optional_value<MirSurfaceState> const&;
    auto preferred_orientation() const -> mir::optional_value<MirOrientationMode> const&;
    auto aux_rect() const -> mir::optional_value<Rectangle> const&;
    auto edge_attachment() const -> mir::optional_value<MirEdgeAttachment> const&;
    auto min_width() const -> mir::optional_value<Width> const&;
    auto min_height() const -> mir::optional_value<Height> const&;
    auto max_width() const -> mir::optional_value<Width> const&;
    auto max_height() const -> mir::optional_value<Height> const&;
    auto width_inc() const -> mir::optional_value<DeltaX> const&;
    auto height_inc() const -> mir::optional_value<DeltaY> const&;
    auto min_aspect() const -> mir::optional_value<AspectRatio> const&;
    auto max_aspect() const -> mir::optional_value<AspectRatio> const&;
    auto streams() const -> mir::optional_value<std::vector<StreamSpecification>> const&;
    auto parent() const -> mir::optional_value<std::weak_ptr<mir::scene::Surface>> const&;
    auto input_shape() const -> mir::optional_value<std::vector<Rectangle>> const&;
    auto input_mode() const -> mir::optional_value<InputReceptionMode> const&;
    auto shell_chrome() const -> mir::optional_value<MirShellChrome> const&;

    auto top_left() -> mir::optional_value<Point>&;
    auto size() -> mir::optional_value<Size>&;
    auto pixel_format() -> mir::optional_value<MirPixelFormat>&;
    auto buffer_usage() -> mir::optional_value<BufferUsage>&;
    auto name() -> mir::optional_value<std::string>&;
    auto output_id() -> mir::optional_value<int>&;
    auto type() -> mir::optional_value<MirSurfaceType>&;
    auto state() -> mir::optional_value<MirSurfaceState>&;
    auto preferred_orientation() -> mir::optional_value<MirOrientationMode>&;
    auto content_id() -> mir::optional_value<BufferStreamId>&;
    auto aux_rect() -> mir::optional_value<Rectangle>&;
    auto edge_attachment() -> mir::optional_value<MirEdgeAttachment>&;
    auto min_width() -> mir::optional_value<Width>&;
    auto min_height() -> mir::optional_value<Height>&;
    auto max_width() -> mir::optional_value<Width>&;
    auto max_height() -> mir::optional_value<Height>&;
    auto width_inc() -> mir::optional_value<DeltaX>&;
    auto height_inc() -> mir::optional_value<DeltaY>&;
    auto min_aspect() -> mir::optional_value<AspectRatio>&;
    auto max_aspect() -> mir::optional_value<AspectRatio>&;
    auto streams() -> mir::optional_value<std::vector<StreamSpecification>>&;
    auto parent() -> mir::optional_value<std::weak_ptr<mir::scene::Surface>>&;
    auto input_shape() -> mir::optional_value<std::vector<Rectangle>>&;
    auto input_mode() -> mir::optional_value<InputReceptionMode>&;
    auto shell_chrome() -> mir::optional_value<MirShellChrome>&;

private:
    struct Self;
    std::unique_ptr<Self> self;
};
}

#endif //MIRAL_WINDOW_SPECIFICATION_H
