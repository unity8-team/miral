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

#ifndef MIRAL_SHELL_CANONICAL_WINDOW_MANAGER_H_
#define MIRAL_SHELL_CANONICAL_WINDOW_MANAGER_H_

#include "spinner/splash.h"

#include <miral/window.h>
#include <miral/window_management_policy.h>

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

    explicit CanonicalWindowManagerPolicy(miral::WindowManagerTools* const tools, SpinnerSplash const& spinner);

    void handle_app_info_updated(Rectangles const& displays) override;

    void handle_displays_updated(Rectangles const& displays) override;

    auto handle_place_new_surface(
        miral::ApplicationInfo const& app_info,
        miral::WindowSpecification const& request_parameters)
        -> miral::WindowSpecification override;

    void handle_new_window(miral::WindowInfo& window_info) override;

    void handle_window_ready(miral::WindowInfo& window_info) override;

    void handle_modify_window(miral::WindowInfo& window_info, miral::WindowSpecification const& modifications) override;

    void handle_delete_window(miral::WindowInfo& window_info) override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    void handle_raise_window(miral::WindowInfo& window_info) override;

    void handle_focus_lost(miral::WindowInfo const& info) override;

    void handle_focus_gained(miral::WindowInfo const& info) override;

private:
    static const int modifier_mask =
        mir_input_event_modifier_alt |
        mir_input_event_modifier_shift |
        mir_input_event_modifier_sym |
        mir_input_event_modifier_ctrl |
        mir_input_event_modifier_meta;

    void drag(Point cursor);
    void click(Point cursor);
    bool resize(Point cursor);
    void toggle(MirSurfaceState state);


    bool resize(miral::Window const& window, Point cursor, Point old_cursor);

    void apply_resize(miral::WindowInfo& window_info, Point new_pos, Size new_size) const;
    void apply_set_state(miral::WindowInfo& window_info, MirSurfaceState value);
    void generate_decorations_for(miral::WindowInfo& window_info);

    miral::WindowManagerTools* const tools;
    SpinnerSplash const spinner;

    Rectangle display_area;
    Point old_cursor{};
    using FullscreenSurfaces = std::set<miral::Window>;

    FullscreenSurfaces fullscreen_surfaces;

    bool resizing = false;
    bool left_resize = false;
    bool top_resize  = false;
};

#endif /* MIRAL_SHELL_CANONICAL_WINDOW_MANAGER_H_ */
