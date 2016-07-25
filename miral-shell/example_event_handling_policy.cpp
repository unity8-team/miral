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

#include "example_event_handling_policy.h"

#include <miral/window_manager_tools.h>

#include <linux/input.h>

#include <limits>

using namespace miral;

void ExampleEventHandlingPolicy::toggle(MirSurfaceState state)
{
    if (auto const window = tools->active_window())
    {
        auto& info = tools->info_for(window);

        if (info.state() == state)
            state = mir_surface_state_restored;

        tools->set_state(info, state);
    }
}

bool ExampleEventHandlingPolicy::resize(Window const& window, Point cursor, Point old_cursor)
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

bool ExampleEventHandlingPolicy::handle_touch_event(MirTouchEvent const* event)
{
    auto const count = mir_touch_event_point_count(event);

    long total_x = 0;
    long total_y = 0;

    for (auto i = 0U; i != count; ++i)
    {
        total_x += mir_touch_event_axis_value(event, i, mir_touch_axis_x);
        total_y += mir_touch_event_axis_value(event, i, mir_touch_axis_y);
    }

    Point cursor{total_x/count, total_y/count};

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

    int touch_pinch_top = std::numeric_limits<int>::max();
    int touch_pinch_left = std::numeric_limits<int>::max();
    int touch_pinch_width = 0;
    int touch_pinch_height = 0;

    for (auto i = 0U; i != count; ++i)
    {
        for (auto j = 0U; j != i; ++j)
        {
            int dx = mir_touch_event_axis_value(event, i, mir_touch_axis_x) -
                     mir_touch_event_axis_value(event, j, mir_touch_axis_x);

            int dy = mir_touch_event_axis_value(event, i, mir_touch_axis_y) -
                     mir_touch_event_axis_value(event, j, mir_touch_axis_y);

            if (touch_pinch_width < dx)
                touch_pinch_width = dx;

            if (touch_pinch_height < dy)
                touch_pinch_height = dy;
        }

        int const x = mir_touch_event_axis_value(event, i, mir_touch_axis_x);

        int const y = mir_touch_event_axis_value(event, i, mir_touch_axis_y);

        if (touch_pinch_top > y)
            touch_pinch_top = y;

        if (touch_pinch_left > x)
            touch_pinch_left = x;
    }

    bool consumes_event = false;
    if (is_drag)
    {
        if (count == 3)
        {
            if (auto window = tools->active_window())
            {
                auto const old_size = window.size();
                auto const delta_width = DeltaX{touch_pinch_width - old_touch_pinch_width};
                auto const delta_height = DeltaY{touch_pinch_height - old_touch_pinch_height};

                auto const delta_x = DeltaX{touch_pinch_left - old_touch_pinch_left};
                auto const delta_y = DeltaY{touch_pinch_top - old_touch_pinch_top};

                auto const new_width = std::max(old_size.width + delta_width, Width{5});
                auto const new_height = std::max(old_size.height + delta_height, Height{5});
                auto const new_pos = window.top_left() + delta_x + delta_y;

                tools->place_and_size(tools->info_for(window), new_pos, {new_width, new_height});
            }
            consumes_event = true;
        }
    }
    else
    {
        if (auto const& window = tools->window_at(cursor))
            tools->select_active_window(window);
    }

    old_cursor = cursor;
    old_touch_pinch_top = touch_pinch_top;
    old_touch_pinch_left = touch_pinch_left;
    old_touch_pinch_width = touch_pinch_width;
    old_touch_pinch_height = touch_pinch_height;
    return consumes_event;
}

bool ExampleEventHandlingPolicy::handle_pointer_event(MirPointerEvent const* event)
{
    auto const action = mir_pointer_event_action(event);
    auto const modifiers = mir_pointer_event_modifiers(event) & modifier_mask;
    Point const cursor{
        mir_pointer_event_axis_value(event, mir_pointer_axis_x),
        mir_pointer_event_axis_value(event, mir_pointer_axis_y)};

    bool consumes_event = false;
    bool is_resize_event = false;

    if (action == mir_pointer_action_button_down)
    {
        if (auto const window = tools->window_at(cursor))
            tools->select_active_window(window);
    }
    else if (action == mir_pointer_action_motion &&
             modifiers == mir_input_event_modifier_alt)
    {
        if (mir_pointer_event_button_state(event, mir_pointer_button_primary))
        {
            if (auto const target = tools->window_at(old_cursor))
            {
                tools->select_active_window(target);
                tools->drag_active_window(cursor - old_cursor);
            }
            consumes_event = true;
        }

        if (mir_pointer_event_button_state(event, mir_pointer_button_tertiary))
        {
            if (!resizing)
                tools->select_active_window(tools->window_at(old_cursor));
            is_resize_event = resize(tools->active_window(), cursor, old_cursor);
            consumes_event = true;
        }
    }

    resizing = is_resize_event;
    old_cursor = cursor;
    return consumes_event;
}

