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
#include "miral/window_specification.h"

#include <mir/geometry/rectangles.h>
#include <mir/optional_value.h>

namespace mir
{
namespace shell { struct SurfaceSpecification; }
}

namespace miral
{
struct WindowInfo
{
    using AspectRatio = WindowSpecification::AspectRatio;

    WindowInfo(Window const& window, WindowSpecification const& params);
    ~WindowInfo();
    WindowInfo(WindowInfo const& that);
    WindowInfo& operator=(WindowInfo const& that);

    bool can_be_active() const;

    bool can_morph_to(MirSurfaceType new_type) const;

    bool must_have_parent() const;

    bool must_not_have_parent() const;

    bool is_visible() const;

    static bool needs_titlebar(MirSurfaceType type);

    void constrain_resize(mir::geometry::Point& requested_pos, mir::geometry::Size& requested_size) const;

    Window window;

    auto type() const -> MirSurfaceType;
    void type(MirSurfaceType type);

    auto state() const -> MirSurfaceState;
    void state(MirSurfaceState state);

    auto restore_rect() const -> mir::geometry::Rectangle;
    void restore_rect(mir::geometry::Rectangle const& restore_rect);

    auto parent() const -> Window;
    void parent(Window const& parent);

    auto children() const -> std::vector <Window> const&;
    void add_child(Window const& child);
    void remove_child(Window const& child);

    auto min_width() const -> mir::geometry::Width;
    void min_width(mir::geometry::Width min_width);

    auto min_height() const -> mir::geometry::Height;
    void min_height(mir::geometry::Height min_height);

    auto max_width() const -> mir::geometry::Width;
    void max_width(mir::geometry::Width max_width);

    auto max_height() const -> mir::geometry::Height;
    void max_height(mir::geometry::Height max_height);

    mir::optional_value<mir::geometry::DeltaX> width_inc;
    mir::optional_value<mir::geometry::DeltaY> height_inc;
    mir::optional_value<AspectRatio> min_aspect;
    mir::optional_value<AspectRatio> max_aspect;
    mir::optional_value<int> output_id;

    /// This can be used by client code to store window manager specific information
    auto userdata() const -> std::shared_ptr<void>;
    void userdata(std::shared_ptr<void> userdata);

private:
    struct Self;
    std::unique_ptr<Self> const self;
};
}

#endif //MIRAL_WINDOW_INFO_H
