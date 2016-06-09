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

#ifndef MIRAL_CANONICAL_WINDOW_MANAGER_H_
#define MIRAL_CANONICAL_WINDOW_MANAGER_H_

#include <miral/window.h>
#include <miral/window_management_policy.h>

#include <mir/geometry/displacement.h>

#include <atomic>
#include <set>

namespace miral
{
using namespace mir::geometry;

// Based on "Mir and Unity: Surfaces, input, and displays (v0.3)"

// standard window management algorithm:
//  o Switch apps: tap or click on the corresponding tile
//  o Move window: Alt-leftmousebutton drag (three finger drag)
//  o Resize window: Alt-middle_button drag (three finger pinch)
//  o Maximize/restore current window (to display size): Alt-F11
//  o Maximize/restore current window (to display height): Shift-F11
//  o Maximize/restore current window (to display width): Ctrl-F11
//  o client requests to maximize, vertically maximize & restore
class CanonicalWindowManagerPolicy  : public WindowManagementPolicy
{
public:

    explicit CanonicalWindowManagerPolicy(WindowManagerTools* const tools);

    auto place_new_surface(
        ApplicationInfo const& app_info,
        WindowSpecification const& request_parameters)
        -> WindowSpecification override;

    void handle_window_ready(WindowInfo& window_info) override;
    void handle_modify_window(WindowInfo& window_info, WindowSpecification const& modifications) override;
    void handle_raise_window(WindowInfo& window_info) override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;
    bool handle_touch_event(MirTouchEvent const* event) override;
    bool handle_pointer_event(MirPointerEvent const* event) override;


    void advise_focus_gained(WindowInfo const& info) override;

    void handle_displays_updated(Rectangles const& displays) override;

protected:
    static const int modifier_mask =
        mir_input_event_modifier_alt |
        mir_input_event_modifier_shift |
        mir_input_event_modifier_sym |
        mir_input_event_modifier_ctrl |
        mir_input_event_modifier_meta;

private:
    void drag(Point cursor);
    void click(Point cursor);
    bool resize(Point cursor);
    void toggle(MirSurfaceState state);


    bool resize(Window const& window, Point cursor, Point old_cursor);

    WindowManagerTools* const tools;

    Point old_cursor{};

    bool resizing = false;
    bool left_resize = false;
    bool top_resize  = false;

    int old_touch_pinch_top = 0;
    int old_touch_pinch_left = 0;
    int old_touch_pinch_width = 0;
    int old_touch_pinch_height = 0;
};
}

#endif /* MIRAL_CANONICAL_WINDOW_MANAGER_H_ */
