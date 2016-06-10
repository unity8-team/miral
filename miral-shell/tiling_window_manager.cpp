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

#include <miral/application_info.h>
#include <miral/window_info.h>
#include <miral/window_manager_tools.h>

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

TilingWindowManagerPolicy::TilingWindowManagerPolicy(WindowManagerTools* const tools, SpinnerSplash const& spinner,
                                                     miral::InternalClientLauncher const& launcher) :
    tools{tools},
    spinner{spinner},
    launcher{launcher}
{
}

void TilingWindowManagerPolicy::click(Point cursor)
{
    auto const window = tools->window_at(cursor);
    tools->select_active_window(window);
}

void TilingWindowManagerPolicy::handle_displays_updated(Rectangles const& displays)
{
    this->displays = displays;
    dirty_tiles = true;
}

void TilingWindowManagerPolicy::resize(Point cursor)
{
    if (auto const application = application_under(cursor))
    {
        if (application == application_under(old_cursor))
        {
            if (auto const window = tools->select_active_window(tools->window_at(old_cursor)))
            {
                resize(window, cursor, old_cursor, tile_for(tools->info_for(application)));
            }
        }
    }
}

auto TilingWindowManagerPolicy::place_new_surface(
    ApplicationInfo const& app_info,
    WindowSpecification const& request_parameters)
    -> WindowSpecification
{
    auto parameters = request_parameters;

    if (app_info.application() != spinner.session())
    {
        Rectangle const& tile = tile_for(app_info);

        if (!parameters.parent().is_set() || !parameters.parent().value().lock())
        {
            if (app_info.windows().empty())
            {
                parameters.top_left() = tile.top_left;
                parameters.size() = tile.size;
            }
            else
            {
                auto top_level_windows = count_if(begin(app_info.windows()), end(app_info.windows()), [this]
                    (Window const& window){ return !tools->info_for(window).parent(); });

                parameters.top_left() = tile.top_left + top_level_windows*Displacement{15, 15};
            }
        }

        clip_to_tile(parameters, tile);
    }

    return parameters;
}

void TilingWindowManagerPolicy::advise_new_window(WindowInfo& window_info)
{
    if (spinner.session() == window_info.window().application())
        dirty_tiles = true;
}

void TilingWindowManagerPolicy::handle_window_ready(WindowInfo& window_info)
{
    tools->select_active_window(window_info.window());
}

namespace
{
template<typename ValueType>
void reset(mir::optional_value<ValueType>& option)
{
    if (option.is_set()) option.consume();
}
}

void TilingWindowManagerPolicy::handle_modify_window(
    miral::WindowInfo& window_info,
    miral::WindowSpecification const& modifications)
{
    auto mods = modifications;

    // filter out changes we don't want the client making
    reset(mods.top_left());
    reset(mods.size());
    reset(mods.output_id());
    reset(mods.edge_attachment());
    reset(mods.min_width());
    reset(mods.min_height());
    reset(mods.max_width());
    reset(mods.max_height());
    reset(mods.width_inc());
    reset(mods.height_inc());
    reset(mods.min_aspect());
    reset(mods.max_aspect());

    if (mods.state().is_set())
    {
        auto state = transform_set_state(window_info, mods.state().consume());
        window_info.window().set_state(state);
    }

    tools->modify_window(window_info, mods);
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
            if (auto const window = tools->select_active_window(tools->window_at(old_cursor)))
            {
                drag(tools->info_for(window), cursor, old_cursor, tile_for(tools->info_for(application)));
            }
        }
    }
}

void TilingWindowManagerPolicy::handle_raise_window(WindowInfo& window_info)
{
    tools-> select_active_window(window_info.window());
}

bool TilingWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
{
    auto const action = mir_keyboard_event_action(event);
    auto const scan_code = mir_keyboard_event_scan_code(event);
    auto const modifiers = mir_keyboard_event_modifiers(event) & modifier_mask;

    if (action == mir_keyboard_action_down && scan_code == KEY_F12 &&
        (modifiers & modifier_mask) == mir_input_event_modifier_alt)
    {
        launcher.launch("Spinner", spinner);
        return true;
    }

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
    if (auto window = tools->active_window())
    {
        auto& window_info = tools->info_for(window);

        if (window_info.state() == state)
            state = mir_surface_state_restored;

        state = transform_set_state(window_info, state);
        window_info.window().set_state(state);
    }
}

auto TilingWindowManagerPolicy::application_under(Point position)
-> Application
{
    return tools->find_application([&, this](ApplicationInfo const& info)
        { return spinner.session() != info.application() && tile_for(info).contains(position);});
}

void TilingWindowManagerPolicy::update_tiles(Rectangles const& displays)
{
    auto applications = tools->count_applications();

    if (spinner.session()) --applications;

    if (applications < 1 || displays.size() < 1) return;

    auto const bounding_rect = displays.bounding_rectangle();

    auto const total_width  = bounding_rect.size.width.as_int();
    auto const total_height = bounding_rect.size.height.as_int();

    auto index = 0;

    tools->for_each_application([&](ApplicationInfo& info)
        {
            if (spinner.session() == info.application())
                return;

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
    for (auto const& window : info.windows())
    {
        if (window)
        {
            auto& window_info = tools->info_for(window);

            if (!window_info.parent())
            {
                auto const new_pos = window.top_left() + (new_tile.top_left - old_tile.top_left);
                auto const offset = new_pos - new_tile.top_left;

                // For now just scale if was filling width/height of tile
                auto const old_size = window.size();
                auto const scaled_width  = old_size.width  == old_tile.size.width  ? new_tile.size.width  : old_size.width;
                auto const scaled_height = old_size.height == old_tile.size.height ? new_tile.size.height : old_size.height;

                auto width  = std::min(new_tile.size.width.as_int()  - offset.dx.as_int(), scaled_width.as_int());
                auto height = std::min(new_tile.size.height.as_int() - offset.dy.as_int(), scaled_height.as_int());

                tools->place_and_size(window_info, new_pos, {width, height});
            }
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

        auto const scale_x = float(new_displacement.dx.as_int())/std::max(1.0f, float(old_displacement.dx.as_int()));
        auto const scale_y = float(new_displacement.dy.as_int())/std::max(1.0f, float(old_displacement.dy.as_int()));

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

void TilingWindowManagerPolicy::advise_focus_gained(WindowInfo const& info)
{
    tools->raise_tree(info.window());

    if (auto const spinner_session = spinner.session())
    {
        auto const& spinner_info = tools->info_for(spinner_session);

        if (spinner_info.windows().size() > 0)
            tools->raise_tree(spinner_info.windows()[0]);
    }
}

void TilingWindowManagerPolicy::advise_new_app(miral::ApplicationInfo& application)
{
    if (spinner.session() == application.application())
        return;

    application.userdata(std::make_shared<TilingWindowManagerPolicyData>());
    dirty_tiles = true;
}

void TilingWindowManagerPolicy::advise_delete_app(miral::ApplicationInfo const& application)
{
    if (spinner.session() == application.application())
        return;

    dirty_tiles = true;
}
void TilingWindowManagerPolicy::advise_end()
{
    if (dirty_tiles)
        update_tiles(displays);

    dirty_tiles = false;
}
