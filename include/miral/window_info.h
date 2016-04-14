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

#ifndef MIRAL_WINDOW_INFO_H
#define MIRAL_WINDOW_INFO_H

#include "miral/window.h"

#include "mir/geometry/rectangles.h"
#include "mir/optional_value.h"

// TODO remove scene::SurfaceCreationParameters from the interface
#include <mir/scene/surface_creation_parameters.h>

namespace mir
{
namespace scene { class Session; }
}

namespace miral
{
// TODO "Opaquify WindowInfo to provide a stable API
struct WindowInfo
{
    WindowInfo(Window const& window, mir::scene::SurfaceCreationParameters const& params);

    bool can_be_active() const;

    bool can_morph_to(MirSurfaceType new_type) const;

    bool must_have_parent() const;

    bool must_not_have_parent() const;

    bool is_visible() const;

    static bool needs_titlebar(MirSurfaceType type);

    void constrain_resize(mir::geometry::Point& requested_pos, mir::geometry::Size& requested_size) const;

    Window window;

    MirSurfaceType type;
    MirSurfaceState state;
    mir::geometry::Rectangle restore_rect;
    Window parent;
    std::vector <Window> children;
    mir::geometry::Width min_width;
    mir::geometry::Height min_height;
    mir::geometry::Width max_width;
    mir::geometry::Height max_height;
    mir::optional_value<mir::geometry::DeltaX> width_inc;
    mir::optional_value<mir::geometry::DeltaY> height_inc;
    mir::optional_value<mir::shell::SurfaceAspectRatio> min_aspect;
    mir::optional_value<mir::shell::SurfaceAspectRatio> max_aspect;
    mir::optional_value<mir::graphics::DisplayConfigurationOutputId> output_id;

    /// This can be used by client code to store window manager specific information
    std::shared_ptr<void> userdata;
};
}

#endif //MIRAL_WINDOW_INFO_H
