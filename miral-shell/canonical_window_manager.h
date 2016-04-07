/*
 * Copyright © 2015-2016 Canonical Ltd.
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
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef CANONICAL_WINDOW_MANAGER_H_
#define CANONICAL_WINDOW_MANAGER_H_

#include "miral/window_management_policy.h"

#include <mir/geometry/displacement.h>

#include <atomic>
#include <set>

using namespace mir::geometry;

// Based on "Mir and Unity: Surfaces, input, and displays (v0.3)"

// standard window management algorithm:
//  o Switch apps: tap or click on the corresponding tile
//  o Move window: Alt-leftmousebutton drag (three finger drag)
//  o Resize window: Alt-middle_button drag (two finger drag)
//  o Maximize/restore current window (to display size): Alt-F11
//  o Maximize/restore current window (to display height): Shift-F11
//  o Maximize/restore current window (to display width): Ctrl-F11
//  o client requests to maximize, vertically maximize & restore
class CanonicalWindowManagerPolicy  : public miral::WindowManagementPolicy
{
public:

    explicit CanonicalWindowManagerPolicy(miral::WindowManagerTools* const tools);

    void handle_session_info_updated(Rectangles const& displays) override;

    void handle_displays_updated(Rectangles const& displays) override;

    auto handle_place_new_surface(
        miral::SessionInfo const& session_info,
        mir::scene::SurfaceCreationParameters const& request_parameters)
    -> mir::scene::SurfaceCreationParameters override;

    void handle_new_surface(miral::SurfaceInfo& surface_info) override;

    void handle_surface_ready(miral::SurfaceInfo& surface_info) override;

    void handle_modify_surface(miral::SurfaceInfo& surface_info, mir::shell::SurfaceSpecification const& modifications) override;

    void handle_delete_surface(miral::SurfaceInfo& surface_info) override;

    auto handle_set_state(miral::SurfaceInfo& surface_info, MirSurfaceState value) -> MirSurfaceState override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    void handle_raise_surface(miral::SurfaceInfo& surface_info) override;

    void generate_decorations_for(miral::SurfaceInfo& surface_info) override;

private:
    static const int modifier_mask =
        mir_input_event_modifier_alt |
        mir_input_event_modifier_shift |
        mir_input_event_modifier_sym |
        mir_input_event_modifier_ctrl |
        mir_input_event_modifier_meta;

    void drag(Point cursor);
    void click(Point cursor);
    void resize(Point cursor);
    void toggle(MirSurfaceState state);

    // "Mir and Unity: Surfaces, input, and displays (v0.3)" talks about active
    //  *window*,but Mir really only understands surfaces
    void select_active_surface(miral::Surface const& surface);
    auto active_surface() const -> miral::Surface;

    bool resize(miral::Surface const& surface, Point cursor, Point old_cursor);
    bool drag(miral::Surface surface, Point to, Point from, Rectangle bounds);
    void move_tree(miral::SurfaceInfo& root, Displacement movement) const;
    void apply_resize(miral::SurfaceInfo& surface_info, Point new_pos, Size new_size) const;
    auto transform_set_state(miral::SurfaceInfo& surface_info, MirSurfaceState value) -> MirSurfaceState;

    miral::WindowManagerTools* const tools;

    Rectangle display_area;
    Point old_cursor{};
    miral::Surface active_surface_;
    using FullscreenSurfaces = std::set<miral::Surface>;

    FullscreenSurfaces fullscreen_surfaces;
};

#endif /* CANONICAL_WINDOW_MANAGER_H_ */
