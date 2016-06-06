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

#include "miral/canonical_window_manager.h"

#include "miral/application_info.h"
#include "miral/window_info.h"
#include "miral/window_manager_tools.h"

#include <linux/input.h>
#include <algorithm>
#include <csignal>

namespace ms = mir::scene;

// Based on "Mir and Unity: Surfaces, input, and displays (v0.3)"

miral::CanonicalWindowManagerPolicy::CanonicalWindowManagerPolicy(WindowManagerTools* const tools) :
    tools{tools}
{
}

void miral::CanonicalWindowManagerPolicy::click(Point cursor)
{
    if (auto const window = tools->window_at(cursor))
        tools->select_active_window(window);
}

void miral::CanonicalWindowManagerPolicy::handle_app_info_updated(Rectangles const& /*displays*/)
{
}

void miral::CanonicalWindowManagerPolicy::handle_displays_updated(Rectangles const& /*displays*/)
{
}

bool miral::CanonicalWindowManagerPolicy::resize(Point cursor)
{
    if (!resizing)
        tools->select_active_window(tools->window_at(old_cursor));
    return resize(tools->active_window(), cursor, old_cursor);
}


auto miral::CanonicalWindowManagerPolicy::place_new_surface(
    miral::ApplicationInfo const& /*app_info*/,
    miral::WindowSpecification const& request_parameters)
    -> miral::WindowSpecification
{
    return request_parameters;
}

void miral::CanonicalWindowManagerPolicy::advise_new_window(WindowInfo& /*window_info*/)
{
}

void miral::CanonicalWindowManagerPolicy::handle_window_ready(WindowInfo& window_info)
{
    tools->select_active_window(window_info.window());
}

void miral::CanonicalWindowManagerPolicy::handle_modify_window(
    WindowInfo& window_info,
    WindowSpecification const& modifications)
{
    tools->modify_window(window_info, modifications);
}

void miral::CanonicalWindowManagerPolicy::advise_delete_window(WindowInfo const& /*window_info*/)
{
}

void miral::CanonicalWindowManagerPolicy::drag(Point cursor)
{
    if (auto const target = tools->window_at(old_cursor))
    {
        tools->select_active_window(target);
        tools->drag_active_window(cursor - old_cursor);
    }
}

void miral::CanonicalWindowManagerPolicy::handle_raise_window(WindowInfo& window_info)
{
    tools->select_active_window(window_info.window());
}

bool miral::CanonicalWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
{
    auto const action = mir_keyboard_event_action(event);
    auto const scan_code = mir_keyboard_event_scan_code(event);
    auto const modifiers = mir_keyboard_event_modifiers(event) & modifier_mask;

    if (action == mir_keyboard_action_down && scan_code == KEY_F11)
    {
        switch (modifiers)
        {
        case mir_input_event_modifier_alt:
            toggle(mir_surface_state_maximized);
            return true;

        case mir_input_event_modifier_shift:
            toggle(mir_surface_state_vertmaximized);
            return true;

        case mir_input_event_modifier_ctrl:
            toggle(mir_surface_state_horizmaximized);
            return true;

        default:
            break;
        }
    }
    else if (action == mir_keyboard_action_down && scan_code == KEY_F4)
    {
        switch (modifiers & modifier_mask)
        {
        case mir_input_event_modifier_alt:
            tools->kill_active_application(SIGTERM);
            return true;

        case mir_input_event_modifier_ctrl:
            if (auto const window = tools->active_window())
                window.request_client_surface_close();

            return true;

        default:
            break;
        }
    }
    else if (action == mir_keyboard_action_down &&
            modifiers == mir_input_event_modifier_alt &&
            scan_code == KEY_TAB)
    {
        tools->focus_next_application();

        return true;
    }
    else if (action == mir_keyboard_action_down &&
            modifiers == mir_input_event_modifier_alt &&
            scan_code == KEY_GRAVE)
    {
        tools->focus_next_within_application();

        return true;
    }

    return false;
}

bool miral::CanonicalWindowManagerPolicy::handle_touch_event(MirTouchEvent const* event)
{
    auto const count = mir_touch_event_point_count(event);

    long total_x = 0;
    long total_y = 0;

    for (auto i = 0U; i != count; ++i)
    {
        total_x += mir_touch_event_axis_value(event, i, mir_touch_axis_x);
        total_y += mir_touch_event_axis_value(event, i, mir_touch_axis_y);
    }

    Point const cursor{total_x/count, total_y/count};

    bool is_drag = true;
    for (auto i = 0U; i != count; ++i)
    {
        switch (mir_touch_event_action(event, i))
        {
        case mir_touch_action_up:
            return false;

        case mir_touch_action_down:
            is_drag = false;

        case mir_touch_action_change:
            continue;
        }
    }

    bool consumes_event = false;
    if (is_drag)
    {
        switch (count)
        {
        case 4:
            resize(cursor);
            consumes_event = true;
            break;

        case 3:
            drag(cursor);
            consumes_event = true;
            break;
        }
    }
    else
    {
        if (auto const& window = tools->window_at(cursor))
            tools->select_active_window(window);
    }

    old_cursor = cursor;
    return consumes_event;
}

bool miral::CanonicalWindowManagerPolicy::handle_pointer_event(MirPointerEvent const* event)
{
    auto const action = mir_pointer_event_action(event);
    auto const modifiers = mir_pointer_event_modifiers(event) & modifier_mask;
    Point const cursor{
        mir_pointer_event_axis_value(event, mir_pointer_axis_x),
        mir_pointer_event_axis_value(event, mir_pointer_axis_y)};

    bool consumes_event = false;
    bool resize_event = false;

    if (action == mir_pointer_action_button_down)
    {
        click(cursor);
    }
    else if (action == mir_pointer_action_motion &&
             modifiers == mir_input_event_modifier_alt)
    {
        if (mir_pointer_event_button_state(event, mir_pointer_button_primary))
        {
            drag(cursor);
            consumes_event = true;
        }

        if (mir_pointer_event_button_state(event, mir_pointer_button_tertiary))
        {
            resize_event = resize(cursor);
            consumes_event = true;
        }
    }

    resizing = resize_event;
    old_cursor = cursor;
    return consumes_event;
}

void miral::CanonicalWindowManagerPolicy::toggle(MirSurfaceState state)
{
    if (auto const window = tools->active_window())
    {
        auto& info = tools->info_for(window);

        if (info.state() == state)
            state = mir_surface_state_restored;

        tools->set_state(info, state);
    }
}

void miral::CanonicalWindowManagerPolicy::advise_focus_gained(WindowInfo const& info)
{
    tools->raise_tree(info.window());
}

void miral::CanonicalWindowManagerPolicy::advise_focus_lost(WindowInfo const& /*info*/)
{
}

bool miral::CanonicalWindowManagerPolicy::resize(Window const& window, Point cursor, Point old_cursor)
{
    if (!window)
        return false;

    auto& window_info = tools->info_for(window);

    auto const top_left = window.top_left();
    Rectangle const old_pos{top_left, window.size()};

    if (!resizing)
    {
        auto anchor = old_pos.bottom_right();

        for (auto const& corner : {
            old_pos.top_right(),
            old_pos.bottom_left(),
            top_left})
        {
            if ((old_cursor - anchor).length_squared() <
                (old_cursor - corner).length_squared())
            {
                anchor = corner;
            }
        }

        left_resize = anchor.x != top_left.x;
        top_resize  = anchor.y != top_left.y;
    }

    int const x_sign = left_resize? -1 : 1;
    int const y_sign = top_resize?  -1 : 1;

    auto delta = cursor-old_cursor;

    auto new_width = old_pos.size.width + x_sign * delta.dx;
    auto new_height = old_pos.size.height + y_sign * delta.dy;

    auto const min_width  = std::max(window_info.min_width(), Width{5});
    auto const min_height = std::max(window_info.min_height(), Height{5});

    if (new_width < min_width)
    {
        new_width = min_width;
        if (delta.dx > DeltaX{0})
            delta.dx = DeltaX{0};
    }

    if (new_height < min_height)
    {
        new_height = min_height;
        if (delta.dy > DeltaY{0})
            delta.dy = DeltaY{0};
    }

    Size new_size{new_width, new_height};
    Point new_pos = top_left + left_resize*delta.dx + top_resize*delta.dy;

    window_info.constrain_resize(new_pos, new_size);
    tools->place_and_size(window_info, new_pos, new_size);

    return true;
}

void miral::CanonicalWindowManagerPolicy::advise_state_change(WindowInfo const& /*window_info*/, MirSurfaceState /*state*/)
{
}

void miral::CanonicalWindowManagerPolicy::advise_resize(WindowInfo const& /*window_info*/, Size const& /*new_size*/)
{
}