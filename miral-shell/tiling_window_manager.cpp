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

#include "tiling_window_manager.h"

#include "miral/application_info.h"
#include "miral/window_info.h"
#include "miral/window_manager_tools.h"

#include <linux/input.h>
#include <algorithm>
#include <csignal>

namespace ms = mir::scene;
using namespace miral;

namespace
{
struct TilingWindowManagerPolicyData
{
    Rectangle tile;
};

inline Rectangle& tile_for(miral::ApplicationInfo& app_info)
{
    return std::static_pointer_cast<TilingWindowManagerPolicyData>(app_info.userdata())->tile;
}

inline Rectangle const& tile_for(miral::ApplicationInfo const& app_info)
{
    return std::static_pointer_cast<TilingWindowManagerPolicyData>(app_info.userdata())->tile;
}
}

// Demonstrate implementing a simple tiling algorithm

TilingWindowManagerPolicy::TilingWindowManagerPolicy(WindowManagerTools* const tools) :
    tools{tools}
{
}

void TilingWindowManagerPolicy::click(Point cursor)
{
    auto const window = tools->window_at(cursor);
    select_active_window(window);
}

void TilingWindowManagerPolicy::handle_app_info_updated(Rectangles const& displays)
{
    update_tiles(displays);
}

void TilingWindowManagerPolicy::handle_displays_updated(Rectangles const& displays)
{
    update_tiles(displays);
}

void TilingWindowManagerPolicy::resize(Point cursor)
{
    if (auto const application = application_under(cursor))
    {
        if (application == application_under(old_cursor))
        {
            if (auto const window = select_active_window(tools->window_at(old_cursor)))
            {
                resize(window, cursor, old_cursor, tile_for(tools->info_for(application)));
            }
        }
    }
}

auto TilingWindowManagerPolicy::handle_place_new_surface(
    ApplicationInfo const& app_info,
    WindowSpecification const& request_parameters)
    -> WindowSpecification
{
    auto parameters = request_parameters;

    Rectangle const& tile = tile_for(app_info);
    parameters.top_left() = parameters.top_left().value() + (tile.top_left - Point{0, 0});

    if (parameters.parent().is_set() && parameters.parent().value().lock())
    {
        auto parent = tools->info_for(parameters.parent().value()).window();
        auto const width = parameters.size().value().width.as_int();
        auto const height = parameters.size().value().height.as_int();

        if (parameters.aux_rect().is_set() && parameters.edge_attachment().is_set())
        {
            auto const edge_attachment = parameters.edge_attachment().value();
            auto const aux_rect = parameters.aux_rect().value();
            auto const parent_top_left = parent.top_left();
            auto const top_left = aux_rect.top_left     -Point{} + parent_top_left;
            auto const top_right= aux_rect.top_right()  -Point{} + parent_top_left;
            auto const bot_left = aux_rect.bottom_left()-Point{} + parent_top_left;

            if (edge_attachment & mir_edge_attachment_vertical)
            {
                if (tile.contains(top_right + Displacement{width, height}))
                {
                    parameters.top_left() = top_right;
                }
                else if (tile.contains(top_left + Displacement{-width, height}))
                {
                    parameters.top_left() = top_left + Displacement{-width, 0};
                }
            }

            if (edge_attachment & mir_edge_attachment_horizontal)
            {
                if (tile.contains(bot_left + Displacement{width, height}))
                {
                    parameters.top_left() = bot_left;
                }
                else if (tile.contains(top_left + Displacement{width, -height}))
                {
                    parameters.top_left() = top_left + Displacement{0, -height};
                }
            }
        }
        else
        {
            auto const parent_top_left = parent.top_left();
            auto const centred = parent_top_left
                                 + 0.5*(as_displacement(parent.size()) - as_displacement(parameters.size().value()))
                                 - DeltaY{(parent.size().height.as_int()-height)/6};

            parameters.top_left() = centred;
        }
    }

    clip_to_tile(parameters, tile);
    return parameters;
}

void TilingWindowManagerPolicy::handle_new_window(WindowInfo& /*window_info*/)
{
}

void TilingWindowManagerPolicy::handle_window_ready(WindowInfo& window_info)
{
    select_active_window(window_info.window());
}

void TilingWindowManagerPolicy::handle_modify_window(
    miral::WindowInfo& window_info,
    miral::WindowSpecification const& modifications)
{
    if (modifications.name().is_set())
        window_info.window().rename(modifications.name().value());
}

void TilingWindowManagerPolicy::handle_delete_window(WindowInfo& /*window_info*/)
{
}

auto TilingWindowManagerPolicy::handle_set_state(WindowInfo& window_info, MirSurfaceState value)
-> MirSurfaceState
{
    auto state = transform_set_state(window_info, value);
    window_info.window().set_state(state);
    return state;
}

auto TilingWindowManagerPolicy::transform_set_state(WindowInfo& window_info, MirSurfaceState value)
-> MirSurfaceState
{
    switch (value)
    {
    case mir_surface_state_restored:
    case mir_surface_state_maximized:
    case mir_surface_state_vertmaximized:
    case mir_surface_state_horizmaximized:
        break;

    default:
        return window_info.state();
    }

    if (window_info.state() == mir_surface_state_restored)
    {
        window_info.restore_rect({window_info.window().top_left(), window_info.window().size()});
    }

    if (window_info.state() == value)
    {
        return window_info.state();
    }

    auto const& tile = tile_for(tools->info_for(window_info.window().application()));

    switch (value)
    {
    case mir_surface_state_restored:
        window_info.window().resize(window_info.restore_rect().size);
        drag(window_info, window_info.restore_rect().top_left, window_info.window().top_left(), tile);
        break;

    case mir_surface_state_maximized:
        window_info.window().resize(tile.size);
        drag(window_info, tile.top_left, window_info.window().top_left(), tile);
        break;

    case mir_surface_state_horizmaximized:
        window_info.window().resize({tile.size.width, window_info.restore_rect().size.height});
        drag(window_info, {tile.top_left.x, window_info.restore_rect().top_left.y}, window_info.window().top_left(), tile);
        break;

    case mir_surface_state_vertmaximized:
        window_info.window().resize({window_info.restore_rect().size.width, tile.size.height});
        drag(window_info, {window_info.restore_rect().top_left.x, tile.top_left.y}, window_info.window().top_left(), tile);
        break;

    default:
        break;
    }

    window_info.state(value);
    return value;
}

void TilingWindowManagerPolicy::drag(Point cursor)
{
    if (auto const application = application_under(cursor))
    {
        if (application == application_under(old_cursor))
        {
            if (auto const window = select_active_window(tools->window_at(old_cursor)))
            {
                drag(tools->info_for(window), cursor, old_cursor, tile_for(tools->info_for(application)));
            }
        }
    }
}

void TilingWindowManagerPolicy::handle_raise_window(WindowInfo& window_info)
{
    select_active_window(window_info.window());
}

bool TilingWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
{
    auto const action = mir_keyboard_event_action(event);
    auto const scan_code = mir_keyboard_event_scan_code(event);
    auto const modifiers = mir_keyboard_event_modifiers(event) & modifier_mask;

    if (action == mir_keyboard_action_down && scan_code == KEY_F11)
    {
        switch (modifiers & modifier_mask)
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
            if (auto const application = tools->focused_application())
                miral::kill(application, SIGTERM);

            return true;

        case mir_input_event_modifier_ctrl:
            if (auto const window = tools->focused_window())
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
        if (auto const prev = tools->focused_window())
        {
            auto const& siblings = tools->info_for(prev.application()).windows();
            auto current = find(begin(siblings), end(siblings), prev);

            while (current != end(siblings) && prev == select_active_window(*current))
                ++current;

            if (current == end(siblings))
            {
                current = begin(siblings);
                while (prev != *current && prev == select_active_window(*current))
                    ++current;
            }
        }

        return true;
    }

    return false;
}

bool TilingWindowManagerPolicy::handle_touch_event(MirTouchEvent const* event)
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
        case 2:
            resize(cursor);
            consumes_event = true;
            break;

        case 3:
            drag(cursor);
            consumes_event = true;
            break;
        }
    }

    old_cursor = cursor;
    return consumes_event;
}

bool TilingWindowManagerPolicy::handle_pointer_event(MirPointerEvent const* event)
{
    auto const action = mir_pointer_event_action(event);
    auto const modifiers = mir_pointer_event_modifiers(event) & modifier_mask;
    Point const cursor{
        mir_pointer_event_axis_value(event, mir_pointer_axis_x),
        mir_pointer_event_axis_value(event, mir_pointer_axis_y)};

    bool consumes_event = false;

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
        else if (mir_pointer_event_button_state(event, mir_pointer_button_tertiary))
        {
            resize(cursor);
            consumes_event = true;
        }
    }

    old_cursor = cursor;
    return consumes_event;
}

void TilingWindowManagerPolicy::toggle(MirSurfaceState state)
{
    if (auto window = tools->focused_window())
    {
        auto& window_info = tools->info_for(window);

        if (window_info.state() == state)
            state = mir_surface_state_restored;

        handle_set_state(window_info, state);
    }
}

auto TilingWindowManagerPolicy::application_under(Point position)
-> Application
{
    return tools->find_application([&](ApplicationInfo const& info) { return tile_for(info).contains(position);});
}

void TilingWindowManagerPolicy::update_tiles(Rectangles const& displays)
{
    auto const applications = tools->count_applications();

    if (applications < 1 || displays.size() < 1) return;

    auto const bounding_rect = displays.bounding_rectangle();

    auto const total_width  = bounding_rect.size.width.as_int();
    auto const total_height = bounding_rect.size.height.as_int();

    auto index = 0;

    tools->for_each_application([&](ApplicationInfo& info)
        {
            if (!info.userdata())
                info.userdata(std::make_shared<TilingWindowManagerPolicyData>());

            auto& tile = tile_for(info);

            auto const x = (total_width * index) / applications;
            ++index;
            auto const dx = (total_width * index) / applications - x;

            auto const old_tile = tile;
            Rectangle const new_tile{{x,  0},
                                     {dx, total_height}};

            update_surfaces(info, old_tile, new_tile);

            tile = new_tile;
        });
}

void TilingWindowManagerPolicy::update_surfaces(ApplicationInfo& info, Rectangle const& old_tile, Rectangle const& new_tile)
{
    auto displacement = new_tile.top_left - old_tile.top_left;

    for (auto& window : info.windows())
    {
        if (window)
        {
            auto const old_pos = window.top_left();
            window.move_to(old_pos + displacement);

            fit_to_new_tile(window, old_tile, new_tile);
        }
    }
}

void TilingWindowManagerPolicy::clip_to_tile(miral::WindowSpecification& parameters, Rectangle const& tile)
{
    auto const displacement = parameters.top_left().value() - tile.top_left;

    auto width = std::min(tile.size.width.as_int()-displacement.dx.as_int(), parameters.size().value().width.as_int());
    auto height = std::min(tile.size.height.as_int()-displacement.dy.as_int(), parameters.size().value().height.as_int());

    parameters.size() = Size{width, height};
}

void TilingWindowManagerPolicy::fit_to_new_tile(miral::Window& window, Rectangle const& old_tile, Rectangle const& new_tile)
{
    auto const displacement = window.top_left() - new_tile.top_left;

    // For now just scale if was filling width/height of tile
    auto const old_size = window.size();
    auto const scaled_width = old_size.width == old_tile.size.width ? new_tile.size.width : old_size.width;
    auto const scaled_height = old_size.height == old_tile.size.height ? new_tile.size.height : old_size.height;

    auto width = std::min(new_tile.size.width.as_int()-displacement.dx.as_int(), scaled_width.as_int());
    auto height = std::min(new_tile.size.height.as_int()-displacement.dy.as_int(), scaled_height.as_int());

    window.resize({width, height});
}

void TilingWindowManagerPolicy::drag(WindowInfo& window_info, Point to, Point from, Rectangle bounds)
{
    if (window_info.window() && window_info.window().input_area_contains(from))
    {
        auto movement = to - from;

        constrained_move(window_info.window(), movement, bounds);

        for (auto const& child: window_info.children())
        {
            auto move = movement;
            constrained_move(child, move, bounds);
        }
    }
}

void TilingWindowManagerPolicy::constrained_move(
    Window window,
    Displacement& movement,
    Rectangle const& bounds)
{
    auto const top_left = window.top_left();
    auto const surface_size = window.size();
    auto const bottom_right = top_left + as_displacement(surface_size);

    if (movement.dx < DeltaX{0})
            movement.dx = std::max(movement.dx, (bounds.top_left - top_left).dx);

    if (movement.dy < DeltaY{0})
            movement.dy = std::max(movement.dy, (bounds.top_left - top_left).dy);

    if (movement.dx > DeltaX{0})
            movement.dx = std::min(movement.dx, (bounds.bottom_right() - bottom_right).dx);

    if (movement.dy > DeltaY{0})
            movement.dy = std::min(movement.dy, (bounds.bottom_right() - bottom_right).dy);

    auto new_pos = window.top_left() + movement;

    window.move_to(new_pos);
}

void TilingWindowManagerPolicy::resize(Window window, Point cursor, Point old_cursor, Rectangle bounds)
{
    if (window && window.input_area_contains(old_cursor))
    {
        auto const top_left = window.top_left();

        auto const old_displacement = old_cursor - top_left;
        auto const new_displacement = cursor - top_left;

        auto const scale_x = new_displacement.dx.as_float()/std::max(1.0f, old_displacement.dx.as_float());
        auto const scale_y = new_displacement.dy.as_float()/std::max(1.0f, old_displacement.dy.as_float());

        if (scale_x <= 0.0f || scale_y <= 0.0f) return;

        auto const old_size = window.size();
        Size new_size{scale_x*old_size.width, scale_y*old_size.height};

        auto const size_limits = as_size(bounds.bottom_right() - top_left);

        if (new_size.width > size_limits.width)
            new_size.width = size_limits.width;

        if (new_size.height > size_limits.height)
            new_size.height = size_limits.height;

        window.resize(new_size);
    }
}

auto TilingWindowManagerPolicy::select_active_window(Window const& window) -> Window
{
    if (!window)
    {
        tools->set_focus_to({});
        return window;
    }

    auto const& info_for = tools->info_for(window);

    if (info_for.can_be_active())
    {
        tools->set_focus_to(info_for.window());
        tools->raise_tree(window);
        return window;
    }
    else
    {
        // Cannot have input focus - try the parent
        if (auto const parent = info_for.parent())
            return select_active_window(parent);

        return {};
    }
}
