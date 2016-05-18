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

#include "canonical_window_manager.h"
#include "titlebar/canonical_window_management_policy_data.h"
#include "spinner/splash.h"

#include "miral/application_info.h"
#include "miral/window_info.h"
#include "miral/window_manager_tools.h"

#include <linux/input.h>
#include <algorithm>
#include <csignal>

namespace ms = mir::scene;
using namespace miral;

// Based on "Mir and Unity: Surfaces, input, and displays (v0.3)"

namespace
{
int const title_bar_height = 10;
Size titlebar_size_for_window(Size window_size)
{
    return {window_size.width, Height{title_bar_height}};
}

Point titlebar_position_for_window(Point window_position)
{
    return {
        window_position.x,
        window_position.y - DeltaY(title_bar_height)
    };
}
}

CanonicalWindowManagerPolicy::CanonicalWindowManagerPolicy(WindowManagerTools* const tools, SpinnerSplash const& spinner) :
    tools{tools}, spinner{spinner}
{
}

void CanonicalWindowManagerPolicy::click(Point cursor)
{
    if (auto const window = tools->window_at(cursor))
        select_active_window(window);
}

void CanonicalWindowManagerPolicy::handle_app_info_updated(Rectangles const& /*displays*/)
{
}

void CanonicalWindowManagerPolicy::handle_displays_updated(Rectangles const& displays)
{
    display_area = displays.bounding_rectangle();

    for (auto window : fullscreen_surfaces)
    {
        if (window)
        {
            auto const& info = tools->info_for(window);
            Rectangle rect{window.top_left(), window.size()};

            tools->place_in_output(info.output_id.value(), rect);
            window.move_to(rect.top_left);
            window.resize(rect.size);
        }
    }
}

bool CanonicalWindowManagerPolicy::resize(Point cursor)
{
    if (!resizing)
        select_active_window(tools->window_at(old_cursor));
    return resize(active_window(), cursor, old_cursor);
}


auto CanonicalWindowManagerPolicy::handle_place_new_surface(
    miral::ApplicationInfo const& app_info,
    miral::WindowSpecification const& request_parameters)
    -> miral::WindowSpecification
{
    auto parameters = request_parameters;
    auto surf_type = parameters.type().is_set() ? parameters.type().value() : mir_surface_type_normal;
    bool const needs_titlebar = WindowInfo::needs_titlebar(surf_type);

    if (needs_titlebar)
        parameters.size() = Size{parameters.size().value().width, parameters.size().value().height + DeltaY{title_bar_height}};

    if (!parameters.state().is_set())
        parameters.state() = mir_surface_state_restored;

    auto const active_display = tools->active_display();

    auto const width = parameters.size().value().width.as_int();
    auto const height = parameters.size().value().height.as_int();

    bool positioned = false;

    bool const has_parent{parameters.parent().is_set() && parameters.parent().value().lock()};

    if (parameters.output_id().is_set() && parameters.output_id().value() != 0)
    {
        Rectangle rect{parameters.top_left().value(), parameters.size().value()};
        tools->place_in_output(parameters.output_id().value(), rect);
        parameters.top_left() = rect.top_left;
        parameters.size() = rect.size;
        parameters.state() = mir_surface_state_fullscreen;
        positioned = true;
    }
    else if (!has_parent) // No parent => client can't suggest positioning
    {
        if (app_info.windows.size() > 0)
        {
            if (auto const default_window = app_info.windows[0])
            {
                static Displacement const offset{title_bar_height, title_bar_height};

                parameters.top_left() = default_window.top_left() + offset;

                Rectangle display_for_app{default_window.top_left(), default_window.size()};

                tools->size_to_output(display_for_app);

                positioned = display_for_app.overlaps(Rectangle{parameters.top_left().value(), parameters.size().value()});
            }
        }
    }

    if (has_parent && parameters.aux_rect().is_set() && parameters.edge_attachment().is_set())
    {
        auto parent = tools->info_for(parameters.parent().value()).window;

        auto const edge_attachment = parameters.edge_attachment().value();
        auto const aux_rect = parameters.aux_rect().value();
        auto const parent_top_left = parent.top_left();
        auto const top_left = aux_rect.top_left     -Point{} + parent_top_left;
        auto const top_right= aux_rect.top_right()  -Point{} + parent_top_left;
        auto const bot_left = aux_rect.bottom_left()-Point{} + parent_top_left;

        if (edge_attachment & mir_edge_attachment_vertical)
        {
            if (active_display.contains(top_right + Displacement{width, height}))
            {
                parameters.top_left() = top_right;
                positioned = true;
            }
            else if (active_display.contains(top_left + Displacement{-width, height}))
            {
                parameters.top_left() = top_left + Displacement{-width, 0};
                positioned = true;
            }
        }

        if (edge_attachment & mir_edge_attachment_horizontal)
        {
            if (active_display.contains(bot_left + Displacement{width, height}))
            {
                parameters.top_left() = bot_left;
                positioned = true;
            }
            else if (active_display.contains(top_left + Displacement{width, -height}))
            {
                parameters.top_left() = top_left + Displacement{0, -height};
                positioned = true;
            }
        }
    }
    else if (has_parent)
    {
        auto parent = tools->info_for(parameters.parent().value()).window;
        //  o Otherwise, if the dialog is not the same as any previous dialog for the
        //    same parent window, and/or it does not have user-customized position:
        //      o It should be optically centered relative to its parent, unless this
        //        would overlap or cover the title bar of the parent.
        //      o Otherwise, it should be cascaded vertically (but not horizontally)
        //        relative to its parent, unless, this would cause at least part of
        //        it to extend into shell space.
        auto const parent_top_left = parent.top_left();
        auto const centred = parent_top_left
                             + 0.5*(as_displacement(parent.size()) - as_displacement(parameters.size().value()))
                             - DeltaY{(parent.size().height.as_int()-height)/6};

        parameters.top_left() = centred;
        positioned = true;
    }

    if (!positioned)
    {
        auto centred = active_display.top_left
                             + 0.5*(as_displacement(active_display.size) - as_displacement(parameters.size().value()))
                             - DeltaY{(active_display.size.height.as_int()-height)/6};

        switch (parameters.state().value())
        {
        case mir_surface_state_fullscreen:
        case mir_surface_state_maximized:
            parameters.top_left() = active_display.top_left;
            parameters.size() = active_display.size;
            break;

        case mir_surface_state_vertmaximized:
            centred.y = active_display.top_left.y;
            parameters.top_left() = centred;
            parameters.size() = Size{parameters.size().value().width, active_display.size.height};
            break;

        case mir_surface_state_horizmaximized:
            centred.x = active_display.top_left.x;
            parameters.top_left() = centred;
            parameters.size() = Size{active_display.size.width, parameters.size().value().height};
            break;

        default:
            parameters.top_left() = centred;
        }

        if (parameters.top_left().value().y < display_area.top_left.y)
            parameters.top_left() = Point{parameters.top_left().value().x, display_area.top_left.y};
    }

    if (parameters.state().value() != mir_surface_state_fullscreen && needs_titlebar)
    {
        parameters.top_left() = Point{parameters.top_left().value().x, parameters.top_left().value().y + DeltaY{title_bar_height}};
        parameters.size() = Size{parameters.size().value().width, parameters.size().value().height - DeltaY{title_bar_height}};
    }

    return parameters;
}

void CanonicalWindowManagerPolicy::generate_decorations_for(WindowInfo& window_info)
{
    Window const& window = window_info.window;

    if (!window_info.needs_titlebar(window_info.type()))
        return;

    auto format = mir_pixel_format_xrgb_8888;
    WindowSpecification params;
    params.size() = titlebar_size_for_window(window.size());
    params.name() = "decoration";
    params.pixel_format() = format;
    params.buffer_usage() = WindowSpecification::BufferUsage::software;
    params.top_left() = titlebar_position_for_window(window.top_left());
    params.type() = mir_surface_type_gloss;

    auto& titlebar_info = tools->build_window(window.application(), params);
    titlebar_info.window.set_alpha(0.9);
    titlebar_info.parent(window);

    auto data = std::make_shared<CanonicalWindowManagementPolicyData>(titlebar_info.window);
    window_info.userdata = data;
    window_info.add_child(titlebar_info.window);
}

void CanonicalWindowManagerPolicy::handle_new_window(WindowInfo& window_info)
{
    if (window_info.state() == mir_surface_state_fullscreen)
        fullscreen_surfaces.insert(window_info.window);

    generate_decorations_for(window_info);
}

void CanonicalWindowManagerPolicy::handle_window_ready(WindowInfo& window_info)
{
    select_active_window(window_info.window);
}

void CanonicalWindowManagerPolicy::handle_modify_window(
    WindowInfo& window_info,
    WindowSpecification const& modifications)
{
    auto window_info_new = window_info;
    auto& window = window_info_new.window;

    if (modifications.parent().is_set())
        window_info_new.parent(tools->info_for(modifications.parent().value()).window);

    if (modifications.type().is_set() &&
        window_info_new.type() != modifications.type().value())
    {
        auto const new_type = modifications.type().value();

        if (!window_info_new.can_morph_to(new_type))
        {
            throw std::runtime_error("Unsupported window type change");
        }

        window_info_new.type(new_type);

        if (window_info_new.must_not_have_parent())
        {
            if (modifications.parent().is_set())
                throw std::runtime_error("Target window type does not support parent");

            window_info_new.parent({});
        }
        else if (window_info_new.must_have_parent())
        {
            if (!window_info_new.parent())
                throw std::runtime_error("Target window type requires parent");
        }

        window.set_type(new_type);
    }

#define COPY_IF_SET(field)\
        if (modifications.field().is_set())\
            window_info_new.field(modifications.field().value())

    COPY_IF_SET(min_width);
    COPY_IF_SET(min_height);
    COPY_IF_SET(max_width);
    COPY_IF_SET(max_height);

#undef COPY_IF_SET

#define COPY_IF_SET(field)\
        if (modifications.field().is_set())\
            window_info_new.field = modifications.field().value()

    COPY_IF_SET(width_inc);
    COPY_IF_SET(height_inc);
    COPY_IF_SET(min_aspect);
    COPY_IF_SET(max_aspect);
    COPY_IF_SET(output_id);

#undef COPY_IF_SET

    std::swap(window_info_new, window_info);

    if (modifications.name().is_set())
        window.rename(modifications.name().value());

    if (modifications.streams().is_set())
        window_info_new.window.configure_streams(modifications.streams().value());

    if (modifications.input_shape().is_set())
        window.set_input_region(modifications.input_shape().value());

    if (modifications.size().is_set())
    {
        auto new_size = modifications.size().value();

        auto top_left = window.top_left();

        window_info.constrain_resize(top_left, new_size);

        apply_resize(window_info, top_left, new_size);
    }

    if (modifications.state().is_set())
        handle_set_state(window_info, modifications.state().value());
}

void CanonicalWindowManagerPolicy::handle_delete_window(WindowInfo& window_info)
{
    fullscreen_surfaces.erase(window_info.window);

    if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(window_info.userdata))
    {
        tools->destroy(titlebar->window);
    }

    active_window_.reset();
}

auto CanonicalWindowManagerPolicy::handle_set_state(WindowInfo& window_info, MirSurfaceState value)
-> MirSurfaceState
{
    auto state = transform_set_state(window_info, value);
    window_info.window.set_state(state);
    return state;
}

auto CanonicalWindowManagerPolicy::transform_set_state(WindowInfo& window_info, MirSurfaceState value)
-> MirSurfaceState
{
    switch (value)
    {
    case mir_surface_state_restored:
    case mir_surface_state_maximized:
    case mir_surface_state_vertmaximized:
    case mir_surface_state_horizmaximized:
    case mir_surface_state_fullscreen:
    case mir_surface_state_hidden:
    case mir_surface_state_minimized:
        break;

    default:
        return window_info.state();
    }

    if (window_info.state() == mir_surface_state_restored)
    {
        window_info.restore_rect({window_info.window.top_left(), window_info.window.size()});
    }

    if (window_info.state() != mir_surface_state_fullscreen)
    {
        window_info.output_id = decltype(window_info.output_id){};
        fullscreen_surfaces.erase(window_info.window);
    }
    else
    {
        fullscreen_surfaces.insert(window_info.window);
    }

    if (window_info.state() == value)
    {
        return window_info.state();
    }

    auto const old_pos = window_info.window.top_left();
    Displacement movement;

    switch (value)
    {
    case mir_surface_state_restored:
        movement = window_info.restore_rect().top_left - old_pos;
        window_info.window.resize(window_info.restore_rect().size);
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(window_info.userdata))
        {
            titlebar->window.resize(titlebar_size_for_window(window_info.restore_rect().size));
            titlebar->window.show();
        }
        break;

    case mir_surface_state_maximized:
        movement = display_area.top_left - old_pos;
        window_info.window.resize(display_area.size);
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(window_info.userdata))
            titlebar->window.hide();
        break;

    case mir_surface_state_horizmaximized:
        movement = Point{display_area.top_left.x, window_info.restore_rect().top_left.y} - old_pos;
        window_info.window.resize({display_area.size.width, window_info.restore_rect().size.height});
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(window_info.userdata))
        {
            titlebar->window.resize(titlebar_size_for_window({display_area.size.width, window_info.restore_rect().size.height}));
            titlebar->window.show();
        }
        break;

    case mir_surface_state_vertmaximized:
        movement = Point{window_info.restore_rect().top_left.x, display_area.top_left.y} - old_pos;
        window_info.window.resize({window_info.restore_rect().size.width, display_area.size.height});
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(window_info.userdata))
            titlebar->window.hide();
        break;

    case mir_surface_state_fullscreen:
    {
        Rectangle rect{old_pos, window_info.window.size()};

        if (window_info.output_id.is_set())
        {
            tools->place_in_output(window_info.output_id.value(), rect);
        }
        else
        {
            tools->size_to_output(rect);
        }

        movement = rect.top_left - old_pos;
        window_info.window.resize(rect.size);
        break;
    }

    case mir_surface_state_hidden:
    case mir_surface_state_minimized:
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(window_info.userdata))
            titlebar->window.hide();
        window_info.window.hide();
        window_info.state(value);
        return value;

    default:
        break;
    }

    // TODO It is rather simplistic to move a tree WRT the top_left of the root
    // TODO when resizing. But for more sophistication we would need to encode
    // TODO some sensible layout rules.
    move_tree(window_info, movement);

    window_info.state(value);

    if (window_info.is_visible())
        window_info.window.show();

    return window_info.state();
}

void CanonicalWindowManagerPolicy::drag(Point cursor)
{
    select_active_window(tools->window_at(old_cursor));
    drag(active_window(), cursor, old_cursor, display_area);
}

void CanonicalWindowManagerPolicy::handle_raise_window(WindowInfo& window_info)
{
    select_active_window(window_info.window);
}

bool CanonicalWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
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
        if (auto const window = tools->focused_window())
            select_active_window(window);

        return true;
    }
    else if (action == mir_keyboard_action_down &&
            modifiers == mir_input_event_modifier_alt &&
            scan_code == KEY_GRAVE)
    {
        if (auto const prev = tools->focused_window())
        {
            auto const& siblings = tools->info_for(prev.application()).windows;
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

bool CanonicalWindowManagerPolicy::handle_touch_event(MirTouchEvent const* event)
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

bool CanonicalWindowManagerPolicy::handle_pointer_event(MirPointerEvent const* event)
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
    else if (action == mir_pointer_action_motion && !modifiers)
    {
        if (mir_pointer_event_button_state(event, mir_pointer_button_primary))
        {
            // TODO this is a rather roundabout way to detect a titlebar
            if (auto const possible_titlebar = tools->window_at(old_cursor))
            {
                if (auto const parent = tools->info_for(possible_titlebar).parent())
                {
                    if (std::static_pointer_cast<CanonicalWindowManagementPolicyData>(tools->info_for(parent).userdata))
                    {
                        drag(cursor);
                        consumes_event = true;
                    }
                }
            }
        }
    }

    resizing = resize_event;
    old_cursor = cursor;
    return consumes_event;
}

void CanonicalWindowManagerPolicy::toggle(MirSurfaceState state)
{
    if (auto window = active_window())
    {
        auto& info = tools->info_for(window);

        if (info.state() == state)
            state = mir_surface_state_restored;

        handle_set_state(info, state);
    }
}

auto CanonicalWindowManagerPolicy::select_active_window(Window const& hint) -> miral::Window
{
    if (hint == active_window_)
        return hint ;

    if (!hint)
    {
        if (auto const active_surface = active_window_)
        {
            auto& info = tools->info_for(active_surface);
            if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(info.userdata))
            {
                titlebar->paint_titlebar(0x3F);
            }
        }

        if (active_window_)
            tools->set_focus_to({});

        active_window_.reset();
        return hint;
    }

    auto const& info_for = tools->info_for(hint);

    if (info_for.can_be_active())
    {
        if (auto const active_surface = active_window_)
        {
            auto& info = tools->info_for(active_surface);
            if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(info.userdata))
            {
                titlebar->paint_titlebar(0x3F);
            }
        }
        auto& info = tools->info_for(hint);
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(info.userdata))
        {
            titlebar->paint_titlebar(0xFF);
        }
        tools->set_focus_to(info_for.window);
        tools->raise_tree(info_for.window);
        active_window_ = info_for.window;

        // Frig to force the spinner to the top
        if (auto const spinner_session = spinner.session())
        {
            auto const& spinner_info = tools->info_for(spinner_session);

            if (spinner_info.windows.size() > 0)
                tools->raise_tree(spinner_info.windows[0]);
        }

        return hint;
    }
    else
    {
        // Cannot have input focus - try the parent
        if (auto const parent = info_for.parent())
            return select_active_window(parent);
    }

    return {};
}

auto CanonicalWindowManagerPolicy::active_window() const
-> Window
{
    return active_window_;
}

bool CanonicalWindowManagerPolicy::resize(Window const& window, Point cursor, Point old_cursor)
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

    apply_resize(window_info, new_pos, new_size);

    return true;
}

void CanonicalWindowManagerPolicy::apply_resize(WindowInfo& window_info, Point new_pos, Size new_size) const
{
    window_info.constrain_resize(new_pos, new_size);

    if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(window_info.userdata))
        titlebar->window.resize({new_size.width, Height{title_bar_height}});

    window_info.window.resize(new_size);

    move_tree(window_info, new_pos-window_info.window.top_left());
}

bool CanonicalWindowManagerPolicy::drag(Window window, Point to, Point from, Rectangle /*bounds*/)
{
    if (!window)
        return false;

    auto& window_info = tools->info_for(window);

    if (!window.input_area_contains(from) &&
        !std::static_pointer_cast<CanonicalWindowManagementPolicyData>(window_info.userdata))
        return false;

    auto movement = to - from;

    // placeholder - constrain onscreen

    switch (window_info.state())
    {
    case mir_surface_state_restored:
        break;

    // "A vertically maximised window is anchored to the top and bottom of
    // the available workspace and can have any width."
    case mir_surface_state_vertmaximized:
        movement.dy = DeltaY(0);
        break;

    // "A horizontally maximised window is anchored to the left and right of
    // the available workspace and can have any height"
    case mir_surface_state_horizmaximized:
        movement.dx = DeltaX(0);
        break;

    // "A maximised window is anchored to the top, bottom, left and right of the
    // available workspace. For example, if the launcher is always-visible then
    // the left-edge of the window is anchored to the right-edge of the launcher."
    case mir_surface_state_maximized:
    case mir_surface_state_fullscreen:
    default:
        return true;
    }

    move_tree(window_info, movement);

    return true;
}

void CanonicalWindowManagerPolicy::move_tree(WindowInfo& root, Displacement movement) const
{
    root.window.move_to(root.window.top_left() + movement);

    for (auto const& child: root.children())
    {
        move_tree(tools->info_for(child), movement);
    }
}
