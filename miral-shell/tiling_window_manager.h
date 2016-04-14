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

#ifndef MIRAL_SHELL_TILING_WINDOW_MANAGER_H
#define MIRAL_SHELL_TILING_WINDOW_MANAGER_H

#include "miral/window_management_policy.h"

using namespace mir::geometry;

// Demonstrate implementing a simple tiling algorithm

// simple tiling algorithm:
//  o Switch apps: tap or click on the corresponding tile
//  o Move window: Alt-leftmousebutton drag (three finger drag)
//  o Resize window: Alt-middle_button drag (two finger drag)
//  o Maximize/restore current window (to tile size): Alt-F11
//  o Maximize/restore current window (to tile height): Shift-F11
//  o Maximize/restore current window (to tile width): Ctrl-F11
//  o client requests to maximize, vertically maximize & restore
class TilingWindowManagerPolicy : public miral::WindowManagementPolicy
{
public:
    explicit TilingWindowManagerPolicy(miral::WindowManagerTools* const tools);

    void handle_session_info_updated(Rectangles const& displays) override;

    void handle_displays_updated(Rectangles const& displays) override;

    auto handle_place_new_surface(
        miral::SessionInfo const& session_info,
        mir::scene::SurfaceCreationParameters const& request_parameters)
    -> mir::scene::SurfaceCreationParameters override;

    void handle_new_surface(miral::WindowInfo& surface_info) override;

    void handle_surface_ready(miral::WindowInfo& surface_info) override;

    void handle_modify_surface(miral::WindowInfo& surface_info, mir::shell::SurfaceSpecification const& modifications) override;

    void handle_delete_surface(miral::WindowInfo& surface_info) override;

    auto handle_set_state(miral::WindowInfo& surface_info, MirSurfaceState value) -> MirSurfaceState override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    void handle_raise_surface(miral::WindowInfo& surface_info) override;

    void generate_decorations_for(miral::WindowInfo& surface_info) override;

private:
    static const int modifier_mask =
        mir_input_event_modifier_alt |
        mir_input_event_modifier_shift |
        mir_input_event_modifier_sym |
        mir_input_event_modifier_ctrl |
        mir_input_event_modifier_meta;

    void click(Point cursor);
    void resize(Point cursor);
    void drag(Point cursor);
    void toggle(MirSurfaceState state);

    miral::Application session_under(Point position);

    void update_tiles(Rectangles const& displays);
    void update_surfaces(miral::SessionInfo& info, Rectangle const& old_tile, Rectangle const& new_tile);
    void drag(miral::WindowInfo& surface_info, Point to, Point from, Rectangle bounds);
    auto select_active_surface(miral::Window const& surface) -> miral::Window;
    auto transform_set_state(miral::WindowInfo& surface_info, MirSurfaceState value) -> MirSurfaceState;

    static void clip_to_tile(mir::scene::SurfaceCreationParameters& parameters, Rectangle const& tile);
    static void fit_to_new_tile(miral::Window& surface, Rectangle const& old_tile, Rectangle const& new_tile);
    static void resize(miral::Window surface, Point cursor, Point old_cursor, Rectangle bounds);
    static void constrained_move(miral::Window surface, Displacement& movement, Rectangle const& bounds);

    miral::WindowManagerTools* const tools;

    Point old_cursor{};
};

#endif /* MIRAL_SHELL_TILING_WINDOW_MANAGER_H */
