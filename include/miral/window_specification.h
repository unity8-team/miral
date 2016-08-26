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

// TODO this wants to move to Mir's toolkit API
// It is inspired by GdkGravity
typedef enum MirPlacementGravity
{
    mir_placement_gravity_northwest = 1,   // the reference point is at the top left corner.
    mir_placement_gravity_north,           // the reference point is in the middle of the top edge.
    mir_placement_gravity_northeast,       // the reference point is at the top right corner.
    mir_placement_gravity_west,            // the reference point is at the middle of the left edge.
    mir_placement_gravity_centre,          // the reference point is at the center of the window.
    mir_placement_gravity_east,            // the reference point is at the middle of the right edge.
    mir_placement_gravity_southwest,       // the reference point is at the lower left corner.
    mir_placement_gravity_south,           // the reference point is at the middle of the lower edge.
    mir_placement_gravity_southeast,       // the reference point is at the lower right corner.
} MirPlacementGravity;

// TODO this wants to move to Mir's toolkit API
// It is inspired by GdkAnchorHints
/**
 * Positioning hints for aligning a window relative to a rectangle.
 *
 * These hints determine how the window should be positioned in the case that
 * the window would fall off-screen if placed in its ideal position.
 *
 * For example, mir_placement_hints_flip_x will invert the x component of
 * aux_rect_placement_offset and replace mir_placement_gravity_northwest with
 * mir_placement_gravity_northeast and vice versa if the window extends
 * beyond the left or right edges of the monitor.
 *
 * If mir_placement_hints_slide_x is set, the window can be shifted
 * horizontally to fit on-screen.
 *
 * If mir_placement_hints_resize_x is set, the window can be shrunken
 * horizontally to fit.
 *
 * When multiple flags are set, flipping should take precedence over sliding,
 * which should take precedence over resizing.
 */
typedef enum MirPlacementHints
{
    mir_placement_hints_flip_x   = 1 << 0,  // allow flipping anchors horizontally
    mir_placement_hints_flip_y   = 1 << 1,  // allow flipping anchors vertically
    mir_placement_hints_slide_x  = 1 << 2,  // allow sliding window horizontally
    mir_placement_hints_slide_y  = 1 << 3,  // allow sliding window vertically
    mir_placement_hints_resize_x = 1 << 4,  // allow resizing window horizontally
    mir_placement_hints_resize_y = 1 << 5,  // allow resizing window vertically

    // allow flipping anchors on both axes
    mir_placement_hints_flip_any = mir_placement_hints_flip_x|mir_placement_hints_flip_y,

    // allow sliding window on both axes
    mir_placement_hints_slide  = mir_placement_hints_slide_x|mir_placement_hints_slide_y,

    // allow resizing window on both axes
    mir_placement_hints_resize = mir_placement_hints_resize_x|mir_placement_hints_resize_y,
} MirPlacementHints;


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
    auto edge_attachment() const -> mir::optional_value<MirEdgeAttachment> const&;  // TODO deprecate
    auto placement_hints() const -> mir::optional_value<MirPlacementHints> const&;
    auto window_placement_gravity() const -> mir::optional_value<MirPlacementGravity> const&;
    auto aux_rect_placement_gravity() const -> mir::optional_value<MirPlacementGravity> const&;
    auto aux_rect_placement_gravity_alt() const -> mir::optional_value<MirPlacementGravity> const&;
    auto aux_rect_placement_offset() const -> mir::optional_value<Displacement> const&;
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
    auto edge_attachment() -> mir::optional_value<MirEdgeAttachment>&;  // TODO deprecate
    auto placement_hints() -> mir::optional_value<MirPlacementHints>&;
    auto window_placement_gravity() -> mir::optional_value<MirPlacementGravity>&;
    auto aux_rect_placement_gravity() -> mir::optional_value<MirPlacementGravity>&;
    auto aux_rect_placement_gravity_alt() -> mir::optional_value<MirPlacementGravity>&;
    auto aux_rect_placement_offset() -> mir::optional_value<Displacement>&;
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
