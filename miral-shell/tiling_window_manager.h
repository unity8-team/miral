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

#include "miral/application.h"
#include "miral/window_management_policy.h"

#include <mir/geometry/displacement.h>

using namespace mir::geometry;

// Demonstrate implementing a simple tiling algorithm

// simple tiling algorithm:
//  o Switch apps: tap or click on the corresponding tile
//  o Move window: Alt-leftmousebutton drag (three finger drag)
//  o Resize window: Alt-middle_button drag (four finger drag)
//  o Maximize/restore current window (to tile size): Alt-F11
//  o Maximize/restore current window (to tile height): Shift-F11
//  o Maximize/restore current window (to tile width): Ctrl-F11
//  o client requests to maximize, vertically maximize & restore
class TilingWindowManagerPolicy : public miral::WindowManagementPolicy
{
public:
    explicit TilingWindowManagerPolicy(miral::WindowManagerTools* const tools);

    auto place_new_surface(
        miral::ApplicationInfo const& app_info,
        miral::WindowSpecification const& request_parameters)
        -> miral::WindowSpecification override;

    void handle_displays_updated(Rectangles const& displays) override;
    void handle_window_ready(miral::WindowInfo& window_info) override;
    void handle_modify_window(miral::WindowInfo& window_info, miral::WindowSpecification const& modifications) override;
    bool handle_keyboard_event(MirKeyboardEvent const* event) override;
    bool handle_touch_event(MirTouchEvent const* event) override;
    bool handle_pointer_event(MirPointerEvent const* event) override;
    void handle_raise_window(miral::WindowInfo& window_info) override;

    void advise_end() override;

    void advise_new_window(miral::WindowInfo& window_info) override;
    void advise_focus_gained(miral::WindowInfo const& info) override;
    void advise_new_app(miral::ApplicationInfo& application) override;
    void advise_delete_app(miral::ApplicationInfo const& application) override;

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

    miral::Application application_under(Point position);

    void update_tiles(Rectangles const& displays);
    void update_surfaces(miral::ApplicationInfo& info, Rectangle const& old_tile, Rectangle const& new_tile);
    void drag(miral::WindowInfo& window_info, Point to, Point from, Rectangle bounds);
    auto transform_set_state(miral::WindowInfo& window_info, MirSurfaceState value) -> MirSurfaceState;

    static void clip_to_tile(miral::WindowSpecification& parameters, Rectangle const& tile);
    static void resize(miral::Window window, Point cursor, Point old_cursor, Rectangle bounds);
    static void constrained_move(miral::Window window, Displacement& movement, Rectangle const& bounds);

    miral::WindowManagerTools* const tools;

    Point old_cursor{};
    Rectangles displays;
    bool dirty_tiles = false;
};

#endif /* MIRAL_SHELL_TILING_WINDOW_MANAGER_H */
