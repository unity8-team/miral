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

#ifndef MIRAL_EXAMPLE_EVENT_HANDLING_POLICY_H
#define MIRAL_EXAMPLE_EVENT_HANDLING_POLICY_H

#include <miral/canonical_window_manager.h>

using namespace mir::geometry;

// example event handling:
//  o Switch apps: Alt+Tab, tap or click on the corresponding window
//  o Switch window: Alt+`, tap or click on the corresponding window
//  o Move window: Alt-leftmousebutton drag (three finger drag)
//  o Resize window: Alt-middle_button drag (three finger pinch)
//  o Maximize/restore current window (to display size): Alt-F11
//  o Maximize/restore current window (to display height): Shift-F11
//  o Maximize/restore current window (to display width): Ctrl-F11
class ExampleEventHandlingPolicy : public miral::CanonicalWindowManagerPolicy
{
public:
    using miral::CanonicalWindowManagerPolicy::CanonicalWindowManagerPolicy;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;
    bool handle_touch_event(MirTouchEvent const* event) override;
    bool handle_pointer_event(MirPointerEvent const* event) override;

private:
    void toggle(MirSurfaceState state);

    bool resize(miral::Window const& window, Point cursor, Point old_cursor);

    Point old_cursor{};

    bool resizing = false;
    bool left_resize = false;
    bool top_resize  = false;

    int old_touch_pinch_top = 0;
    int old_touch_pinch_left = 0;
    int old_touch_pinch_width = 0;
    int old_touch_pinch_height = 0;
};

#endif //MIRAL_EXAMPLE_EVENT_HANDLING_POLICY_H
