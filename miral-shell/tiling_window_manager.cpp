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

#include "miral/session.h"
#include "miral/window_manager_tools.h"

#include <linux/input.h>
#include <csignal>

namespace ms = mir::scene;
using namespace miral;

namespace
{
struct TilingWindowManagerPolicyData
{
    Rectangle tile;
};

inline Rectangle& tile_for(miral::SessionInfo& session_info)
{
    return std::static_pointer_cast<TilingWindowManagerPolicyData>(session_info.userdata)->tile;
}

inline Rectangle const& tile_for(miral::SessionInfo const& session_info)
{
    return std::static_pointer_cast<TilingWindowManagerPolicyData>(session_info.userdata)->tile;
}
}

// Demonstrate implementing a simple tiling algorithm

TilingWindowManagerPolicy::TilingWindowManagerPolicy(WindowManagerTools* const tools) :
    tools{tools}
{
}

void TilingWindowManagerPolicy::click(Point cursor)
{
    auto const session = session_under(cursor);
    auto const surface = tools->surface_at(cursor);
    select_active_surface(surface);
}

void TilingWindowManagerPolicy::handle_session_info_updated(Rectangles const& displays)
{
    update_tiles(displays);
}

void TilingWindowManagerPolicy::handle_displays_updated(Rectangles const& displays)
{
    update_tiles(displays);
}

void TilingWindowManagerPolicy::resize(Point cursor)
{
    if (auto const session = session_under(cursor))
    {
        if (session == session_under(old_cursor))
        {
            if (auto const surface = select_active_surface(tools->surface_at(old_cursor)))
            {
                resize(surface, cursor, old_cursor, tile_for(tools->info_for(session)));
            }
        }
    }
}

auto TilingWindowManagerPolicy::handle_place_new_surface(
    SessionInfo const& session_info,
    ms::SurfaceCreationParameters const& request_parameters)
-> ms::SurfaceCreationParameters
{
    auto parameters = request_parameters;

    Rectangle const& tile = tile_for(session_info);
    parameters.top_left = parameters.top_left + (tile.top_left - Point{0, 0});

    if (parameters.parent.lock())
    {
        auto parent = tools->info_for(parameters.parent).surface;
        auto const width = parameters.size.width.as_int();
        auto const height = parameters.size.height.as_int();

        if (parameters.aux_rect.is_set() && parameters.edge_attachment.is_set())
        {
            auto const edge_attachment = parameters.edge_attachment.value();
            auto const aux_rect = parameters.aux_rect.value();
            auto const parent_top_left = parent.top_left();
            auto const top_left = aux_rect.top_left     -Point{} + parent_top_left;
            auto const top_right= aux_rect.top_right()  -Point{} + parent_top_left;
            auto const bot_left = aux_rect.bottom_left()-Point{} + parent_top_left;

            if (edge_attachment & mir_edge_attachment_vertical)
            {
                if (tile.contains(top_right + Displacement{width, height}))
                {
                    parameters.top_left = top_right;
                }
                else if (tile.contains(top_left + Displacement{-width, height}))
                {
                    parameters.top_left = top_left + Displacement{-width, 0};
                }
            }

            if (edge_attachment & mir_edge_attachment_horizontal)
            {
                if (tile.contains(bot_left + Displacement{width, height}))
                {
                    parameters.top_left = bot_left;
                }
                else if (tile.contains(top_left + Displacement{width, -height}))
                {
                    parameters.top_left = top_left + Displacement{0, -height};
                }
            }
        }
        else
        {
            auto const parent_top_left = parent.top_left();
            auto const centred = parent_top_left
                                 + 0.5*(as_displacement(parent.size()) - as_displacement(parameters.size))
                                 - DeltaY{(parent.size().height.as_int()-height)/6};

            parameters.top_left = centred;
        }
    }

    clip_to_tile(parameters, tile);
    return parameters;
}

void TilingWindowManagerPolicy::generate_decorations_for(SurfaceInfo& /*surface_info*/)
{
}

void TilingWindowManagerPolicy::handle_new_surface(SurfaceInfo& surface_info)
{
    auto const surface = surface_info.surface;
    auto const session = surface.session();

    tools->info_for(session).surfaces.push_back(surface);

    if (auto const parent = surface_info.parent)
    {
        tools->info_for(parent).children.push_back(surface);
    }
}

void TilingWindowManagerPolicy::handle_surface_ready(SurfaceInfo& surface_info)
{
    select_active_surface(surface_info.surface);
}

void TilingWindowManagerPolicy::handle_modify_surface(
    SurfaceInfo& surface_info,
    mir::shell::SurfaceSpecification const& modifications)
{
    if (modifications.name.is_set())
        surface_info.surface.rename(modifications.name.value());
}

void TilingWindowManagerPolicy::handle_delete_surface(SurfaceInfo& surface_info)
{
    auto const session = surface_info.surface.session();
    auto const& surface = surface_info.surface;

    if (auto const parent = surface_info.parent)
    {
        auto& siblings = tools->info_for(parent).children;

        for (auto i = begin(siblings); i != end(siblings); ++i)
        {
            if (surface == *i)
            {
                siblings.erase(i);
                break;
            }
        }
    }

    auto& surfaces = tools->info_for(session).surfaces;

    for (auto i = begin(surfaces); i != end(surfaces); ++i)
    {
        if (surface == *i)
        {
            surfaces.erase(i);
            break;
        }
    }

    surface_info.surface.destroy_surface();

    if (surfaces.empty() && session == tools->focused_session())
    {
        tools->focus_next_session();
        select_active_surface(tools->focused_surface());
    }
}

auto TilingWindowManagerPolicy::handle_set_state(SurfaceInfo& surface_info, MirSurfaceState value)
-> MirSurfaceState
{
    auto state = transform_set_state(surface_info, value);
    surface_info.surface.set_state(state);
    return state;
}

auto TilingWindowManagerPolicy::transform_set_state(SurfaceInfo& surface_info, MirSurfaceState value)
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
        return surface_info.state;
    }

    if (surface_info.state == mir_surface_state_restored)
    {
        surface_info.restore_rect = {surface_info.surface.top_left(), surface_info.surface.size()};
    }

    if (surface_info.state == value)
    {
        return surface_info.state;
    }

    auto const& tile = tile_for(tools->info_for(surface_info.surface.session()));

    switch (value)
    {
    case mir_surface_state_restored:
        surface_info.surface.resize(surface_info.restore_rect.size);
        drag(surface_info, surface_info.restore_rect.top_left, surface_info.surface.top_left(), tile);
        break;

    case mir_surface_state_maximized:
        surface_info.surface.resize(tile.size);
        drag(surface_info, tile.top_left, surface_info.surface.top_left(), tile);
        break;

    case mir_surface_state_horizmaximized:
        surface_info.surface.resize({tile.size.width, surface_info.restore_rect.size.height});
        drag(surface_info, {tile.top_left.x, surface_info.restore_rect.top_left.y}, surface_info.surface.top_left(), tile);
        break;

    case mir_surface_state_vertmaximized:
        surface_info.surface.resize({surface_info.restore_rect.size.width, tile.size.height});
        drag(surface_info, {surface_info.restore_rect.top_left.x, tile.top_left.y}, surface_info.surface.top_left(), tile);
        break;

    default:
        break;
    }

    return surface_info.state = value;
}

void TilingWindowManagerPolicy::drag(Point cursor)
{
    if (auto const session = session_under(cursor))
    {
        if (session == session_under(old_cursor))
        {
            if (auto const surface = select_active_surface(tools->surface_at(old_cursor)))
            {
                drag(tools->info_for(surface), cursor, old_cursor, tile_for(tools->info_for(session)));
            }
        }
    }
}

void TilingWindowManagerPolicy::handle_raise_surface(SurfaceInfo& surface_info)
{
    select_active_surface(surface_info.surface);
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
        if (auto const session = tools->focused_session())
        {
            switch (modifiers & modifier_mask)
            {
            case mir_input_event_modifier_alt:
                session.kill(SIGTERM);
                return true;

            case mir_input_event_modifier_ctrl:
                if (auto const surf = session.default_surface())
                {
                    surf.request_client_surface_close();
                    return true;
                }

            default:
                break;
            }
        }
    }
    else if (action == mir_keyboard_action_down &&
            modifiers == mir_input_event_modifier_alt &&
            scan_code == KEY_TAB)
    {
        tools->focus_next_session();
        select_active_surface(tools->focused_surface());

        return true;
    }
    else if (action == mir_keyboard_action_down &&
            modifiers == mir_input_event_modifier_alt &&
            scan_code == KEY_GRAVE)
    {
        if (auto const prev = tools->focused_surface())
        {
            if (auto const app = tools->focused_session())
                if (auto const surface = app.surface_after(prev))
                {
                    select_active_surface(tools->info_for(surface).surface);
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
    if (auto const session = tools->focused_session())
    {
        if (auto surface = session.default_surface())
        {
            if (surface.state() == state)
                state = mir_surface_state_restored;

            handle_set_state(tools->info_for(surface), state);
        }
    }
}

auto TilingWindowManagerPolicy::session_under(Point position)
-> Session
{
    return tools->find_session([&](SessionInfo const& info) { return tile_for(info).contains(position);});
}

void TilingWindowManagerPolicy::update_tiles(Rectangles const& displays)
{
    auto const sessions = tools->count_sessions();

    if (sessions < 1 || displays.size() < 1) return;

    auto const bounding_rect = displays.bounding_rectangle();

    auto const total_width  = bounding_rect.size.width.as_int();
    auto const total_height = bounding_rect.size.height.as_int();

    auto index = 0;

    tools->for_each_session([&](SessionInfo& info)
    {
        if (!info.userdata)
            info.userdata = std::make_shared<TilingWindowManagerPolicyData>();

        auto& tile = tile_for(info);

        auto const x = (total_width*index)/sessions;
        ++index;
        auto const dx = (total_width*index)/sessions - x;

        auto const old_tile = tile;
        Rectangle const new_tile{{x, 0}, {dx, total_height}};

        update_surfaces(info, old_tile, new_tile);

        tile = new_tile;
    });
}

void TilingWindowManagerPolicy::update_surfaces(SessionInfo& info, Rectangle const& old_tile, Rectangle const& new_tile)
{
    auto displacement = new_tile.top_left - old_tile.top_left;

    for (auto& surface : info.surfaces)
    {
        if (surface)
        {
            auto const old_pos = surface.top_left();
            surface.move_to(old_pos + displacement);

            fit_to_new_tile(surface, old_tile, new_tile);
        }
    }
}

void TilingWindowManagerPolicy::clip_to_tile(ms::SurfaceCreationParameters& parameters, Rectangle const& tile)
{
    auto const displacement = parameters.top_left - tile.top_left;

    auto width = std::min(tile.size.width.as_int()-displacement.dx.as_int(), parameters.size.width.as_int());
    auto height = std::min(tile.size.height.as_int()-displacement.dy.as_int(), parameters.size.height.as_int());

    parameters.size = Size{width, height};
}

void TilingWindowManagerPolicy::fit_to_new_tile(miral::Surface& surface, Rectangle const& old_tile, Rectangle const& new_tile)
{
    auto const displacement = surface.top_left() - new_tile.top_left;

    // For now just scale if was filling width/height of tile
    auto const old_size = surface.size();
    auto const scaled_width = old_size.width == old_tile.size.width ? new_tile.size.width : old_size.width;
    auto const scaled_height = old_size.height == old_tile.size.height ? new_tile.size.height : old_size.height;

    auto width = std::min(new_tile.size.width.as_int()-displacement.dx.as_int(), scaled_width.as_int());
    auto height = std::min(new_tile.size.height.as_int()-displacement.dy.as_int(), scaled_height.as_int());

    surface.resize({width, height});
}

void TilingWindowManagerPolicy::drag(SurfaceInfo& surface_info, Point to, Point from, Rectangle bounds)
{
    if (surface_info.surface && surface_info.surface.input_area_contains(from))
    {
        auto movement = to - from;

        constrained_move(surface_info.surface, movement, bounds);

        for (auto const& child: surface_info.children)
        {
            auto move = movement;
            constrained_move(child, move, bounds);
        }
    }
}

void TilingWindowManagerPolicy::constrained_move(
    Surface surface,
    Displacement& movement,
    Rectangle const& bounds)
{
    auto const top_left = surface.top_left();
    auto const surface_size = surface.size();
    auto const bottom_right = top_left + as_displacement(surface_size);

    if (movement.dx < DeltaX{0})
            movement.dx = std::max(movement.dx, (bounds.top_left - top_left).dx);

    if (movement.dy < DeltaY{0})
            movement.dy = std::max(movement.dy, (bounds.top_left - top_left).dy);

    if (movement.dx > DeltaX{0})
            movement.dx = std::min(movement.dx, (bounds.bottom_right() - bottom_right).dx);

    if (movement.dy > DeltaY{0})
            movement.dy = std::min(movement.dy, (bounds.bottom_right() - bottom_right).dy);

    auto new_pos = surface.top_left() + movement;

    surface.move_to(new_pos);
}

void TilingWindowManagerPolicy::resize(Surface surface, Point cursor, Point old_cursor, Rectangle bounds)
{
    if (surface && surface.input_area_contains(old_cursor))
    {
        auto const top_left = surface.top_left();

        auto const old_displacement = old_cursor - top_left;
        auto const new_displacement = cursor - top_left;

        auto const scale_x = new_displacement.dx.as_float()/std::max(1.0f, old_displacement.dx.as_float());
        auto const scale_y = new_displacement.dy.as_float()/std::max(1.0f, old_displacement.dy.as_float());

        if (scale_x <= 0.0f || scale_y <= 0.0f) return;

        auto const old_size = surface.size();
        Size new_size{scale_x*old_size.width, scale_y*old_size.height};

        auto const size_limits = as_size(bounds.bottom_right() - top_left);

        if (new_size.width > size_limits.width)
            new_size.width = size_limits.width;

        if (new_size.height > size_limits.height)
            new_size.height = size_limits.height;

        surface.resize(new_size);
    }
}

auto TilingWindowManagerPolicy::select_active_surface(Surface const& surface) -> Surface
{
    if (!surface)
    {
        tools->set_focus_to({});
        return surface;
    }

    auto const& info_for = tools->info_for(surface);

    if (info_for.can_be_active())
    {
        tools->set_focus_to(info_for.surface);
        tools->raise_tree(surface);
        return surface;
    }
    else
    {
        // Cannot have input focus - try the parent
        if (auto const parent = info_for.parent)
            return select_active_surface(parent);

        return {};
    }
}