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
#include "miral/session.h"

#include "mir/graphics/buffer.h"
#include "mir/scene/session.h"
#include "mir/scene/surface.h"
#include "mir/scene/surface_creation_parameters.h"
#include "mir/shell/display_layout.h"

#include <linux/input.h>
#include <csignal>
#include <algorithm>

namespace me = mir::examples;
namespace ms = mir::scene;
using namespace mir::geometry;

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

class CanonicalWindowManagementPolicyData
{
public:
    CanonicalWindowManagementPolicyData(miral::Surface surface) : surface{surface} {}
    void paint_titlebar(int intensity);

    miral::Surface surface;

private:
    struct StreamPainter;
    struct AllocatingPainter;
    struct SwappingPainter;

    std::shared_ptr <StreamPainter> stream_painter;
};
}


struct CanonicalWindowManagementPolicyData::StreamPainter
{
    virtual void paint(int) = 0;
    virtual ~StreamPainter() = default;
    StreamPainter() = default;
    StreamPainter(StreamPainter const&) = delete;
    StreamPainter& operator=(StreamPainter const&) = delete;
};

struct CanonicalWindowManagementPolicyData::SwappingPainter
    : CanonicalWindowManagementPolicyData::StreamPainter
{
    SwappingPainter(std::shared_ptr<mir::frontend::BufferStream> const& buffer_stream) :
        buffer_stream{buffer_stream}, buffer{nullptr}
    {
        swap_buffers();
    }

    void swap_buffers()
    {
        auto const callback = [this](mir::graphics::Buffer* new_buffer)
            {
            buffer.store(new_buffer);
            };

        buffer_stream->swap_buffers(buffer, callback);
    }

    void paint(int intensity) override
    {
        if (auto const buf = buffer.load())
        {
            auto const format = buffer_stream->pixel_format();
            auto const sz = buf->size().height.as_int() *
                            buf->size().width.as_int() * MIR_BYTES_PER_PIXEL(format);
            std::vector<unsigned char> pixels(sz, intensity);
            buf->write(pixels.data(), sz);
            swap_buffers();
        }
    }

    std::shared_ptr<mir::frontend::BufferStream> const buffer_stream;
    std::atomic<mir::graphics::Buffer*> buffer;
};

struct CanonicalWindowManagementPolicyData::AllocatingPainter
    : CanonicalWindowManagementPolicyData::StreamPainter
{
    AllocatingPainter(std::shared_ptr<mir::frontend::BufferStream> const& buffer_stream, Size size) :
        buffer_stream(buffer_stream),
        properties({
                       size,
                       buffer_stream->pixel_format(),
                       mir::graphics::BufferUsage::software
                   }),
        front_buffer(buffer_stream->allocate_buffer(properties)),
        back_buffer(buffer_stream->allocate_buffer(properties))
    {
    }

    void paint(int intensity) override
    {
        buffer_stream->with_buffer(back_buffer,
                                   [this, intensity](mir::graphics::Buffer& buffer)
                                       {
                                       auto const format = buffer.pixel_format();
                                       auto const sz = buffer.size().height.as_int() *
                                                       buffer.size().width.as_int() * MIR_BYTES_PER_PIXEL(format);
                                       std::vector<unsigned char> pixels(sz, intensity);
                                       buffer.write(pixels.data(), sz);
                                       buffer_stream->swap_buffers(&buffer, [](mir::graphics::Buffer*){});
                                       });
        std::swap(front_buffer, back_buffer);
    }

    ~AllocatingPainter()
    {
        buffer_stream->remove_buffer(front_buffer);
        buffer_stream->remove_buffer(back_buffer);
    }

    std::shared_ptr<mir::frontend::BufferStream> const buffer_stream;
    mir::graphics::BufferProperties properties;
    mir::graphics::BufferID front_buffer;
    mir::graphics::BufferID back_buffer;
};

void CanonicalWindowManagementPolicyData::paint_titlebar(int intensity)
{
    if (!stream_painter)
    {
        auto stream = std::shared_ptr<mir::scene::Surface>(surface)->primary_buffer_stream();
        try
        {
            stream_painter = std::make_shared<AllocatingPainter>(stream, surface.size());
        }
        catch (...)
        {
            stream_painter = std::make_shared<SwappingPainter>(stream);
        }
    }

    stream_painter->paint(intensity);
}

me::CanonicalWindowManagerPolicy::CanonicalWindowManagerPolicy(
    WindowManagerTools* const tools,
    std::shared_ptr<shell::DisplayLayout> const& display_layout) :
    tools{tools},
    display_layout{display_layout}
{
}

void me::CanonicalWindowManagerPolicy::click(Point cursor)
{
    if (auto const surface = tools->surface_at(cursor))
        select_active_surface(surface);
}

void me::CanonicalWindowManagerPolicy::handle_session_info_updated(Rectangles const& /*displays*/)
{
}

void me::CanonicalWindowManagerPolicy::handle_displays_updated(Rectangles const& displays)
{
    display_area = displays.bounding_rectangle();

    for (auto surface : fullscreen_surfaces)
    {
        if (surface)
        {
            auto const& info = tools->info_for(surface);
            Rectangle rect{surface.top_left(), surface.size()};

            display_layout->place_in_output(info.output_id.value(), rect);
            surface.move_to(rect.top_left);
            surface.resize(rect.size);
        }
    }
}

void me::CanonicalWindowManagerPolicy::resize(Point cursor)
{
    select_active_surface(tools->surface_at(old_cursor));
    resize(active_surface(), cursor, old_cursor);
}

auto me::CanonicalWindowManagerPolicy::handle_place_new_surface(
    std::shared_ptr<ms::Session> const& session,
    ms::SurfaceCreationParameters const& request_parameters)
-> ms::SurfaceCreationParameters
{
    auto parameters = request_parameters;
    auto surf_type = parameters.type.is_set() ? parameters.type.value() : mir_surface_type_normal;
    bool const needs_titlebar = SurfaceInfo::needs_titlebar(surf_type);

    if (needs_titlebar)
        parameters.size.height = parameters.size.height + DeltaY{title_bar_height};

    if (!parameters.state.is_set())
        parameters.state = mir_surface_state_restored;

    auto const active_display = tools->active_display();

    auto const width = parameters.size.width.as_int();
    auto const height = parameters.size.height.as_int();

    bool positioned = false;

    auto const parent = parameters.parent.lock();

    if (parameters.output_id != mir::graphics::DisplayConfigurationOutputId{0})
    {
        Rectangle rect{parameters.top_left, parameters.size};
        display_layout->place_in_output(parameters.output_id, rect);
        parameters.top_left = rect.top_left;
        parameters.size = rect.size;
        parameters.state = mir_surface_state_fullscreen;
        positioned = true;
    }
    else if (!parent) // No parent => client can't suggest positioning
    {
        if (auto const default_surface = session->default_surface())
        {
            static Displacement const offset{title_bar_height, title_bar_height};

            parameters.top_left = default_surface->top_left() + offset;

            geometry::Rectangle display_for_app{default_surface->top_left(), default_surface->size()};

            display_layout->size_to_output(display_for_app);

            positioned = display_for_app.overlaps(Rectangle{parameters.top_left, parameters.size});
        }
    }

    if (parent && parameters.aux_rect.is_set() && parameters.edge_attachment.is_set())
    {
        auto const edge_attachment = parameters.edge_attachment.value();
        auto const aux_rect = parameters.aux_rect.value();
        auto const parent_top_left = parent->top_left();
        auto const top_left = aux_rect.top_left     -Point{} + parent_top_left;
        auto const top_right= aux_rect.top_right()  -Point{} + parent_top_left;
        auto const bot_left = aux_rect.bottom_left()-Point{} + parent_top_left;

        if (edge_attachment & mir_edge_attachment_vertical)
        {
            if (active_display.contains(top_right + Displacement{width, height}))
            {
                parameters.top_left = top_right;
                positioned = true;
            }
            else if (active_display.contains(top_left + Displacement{-width, height}))
            {
                parameters.top_left = top_left + Displacement{-width, 0};
                positioned = true;
            }
        }

        if (edge_attachment & mir_edge_attachment_horizontal)
        {
            if (active_display.contains(bot_left + Displacement{width, height}))
            {
                parameters.top_left = bot_left;
                positioned = true;
            }
            else if (active_display.contains(top_left + Displacement{width, -height}))
            {
                parameters.top_left = top_left + Displacement{0, -height};
                positioned = true;
            }
        }
    }
    else if (parent)
    {
        //  o Otherwise, if the dialog is not the same as any previous dialog for the
        //    same parent window, and/or it does not have user-customized position:
        //      o It should be optically centered relative to its parent, unless this
        //        would overlap or cover the title bar of the parent.
        //      o Otherwise, it should be cascaded vertically (but not horizontally)
        //        relative to its parent, unless, this would cause at least part of
        //        it to extend into shell space.
        auto const parent_top_left = parent->top_left();
        auto const centred = parent_top_left
             + 0.5*(as_displacement(parent->size()) - as_displacement(parameters.size))
             - DeltaY{(parent->size().height.as_int()-height)/6};

        parameters.top_left = centred;
        positioned = true;
    }

    if (!positioned)
    {
        auto const centred = active_display.top_left
            + 0.5*(as_displacement(active_display.size) - as_displacement(parameters.size))
            - DeltaY{(active_display.size.height.as_int()-height)/6};

        switch (parameters.state.value())
        {
        case mir_surface_state_fullscreen:
        case mir_surface_state_maximized:
            parameters.top_left = active_display.top_left;
            parameters.size = active_display.size;
            break;

        case mir_surface_state_vertmaximized:
            parameters.top_left = centred;
            parameters.top_left.y = active_display.top_left.y;
            parameters.size.height = active_display.size.height;
            break;

        case mir_surface_state_horizmaximized:
            parameters.top_left = centred;
            parameters.top_left.x = active_display.top_left.x;
            parameters.size.width = active_display.size.width;
            break;

        default:
            parameters.top_left = centred;
        }

        if (parameters.top_left.y < display_area.top_left.y)
            parameters.top_left.y = display_area.top_left.y;
    }

    if (parameters.state != mir_surface_state_fullscreen && needs_titlebar)
    {
        parameters.top_left.y = parameters.top_left.y + DeltaY{title_bar_height};
        parameters.size.height = parameters.size.height - DeltaY{title_bar_height};
    }

    return parameters;
}

void me::CanonicalWindowManagerPolicy::generate_decorations_for(SurfaceInfo& surface_info)
{
    Surface const& surface = surface_info.surface;

    if (!SurfaceInfo::needs_titlebar(surface.type()))
        return;

    auto format = mir_pixel_format_xrgb_8888;
    ms::SurfaceCreationParameters params;
    params.of_size(titlebar_size_for_window(surface.size()))
        .of_name("decoration")
        .of_pixel_format(format)
        .of_buffer_usage(mir::graphics::BufferUsage::software)
        .of_position(titlebar_position_for_window(surface.top_left()))
        .of_type(mir_surface_type_gloss);

    auto& titlebar_info = tools->build_surface(surface.session(), params);
    titlebar_info.surface.set_alpha(0.9);
    titlebar_info.parent = surface;

    auto data = std::make_shared<CanonicalWindowManagementPolicyData>(titlebar_info.surface);
    surface_info.userdata = data;
    surface_info.children.push_back(titlebar_info.surface);
}

void me::CanonicalWindowManagerPolicy::handle_new_surface(SurfaceInfo& surface_info)
{
    auto const surface = surface_info.surface;
    auto const session = surface_info.surface.session();

    if (auto const parent = surface_info.parent)
    {
        tools->info_for(parent).children.push_back(surface_info.surface);
    }

    tools->info_for(session).surfaces.push_back(surface_info.surface);

    if (surface_info.state == mir_surface_state_fullscreen)
        fullscreen_surfaces.insert(surface_info.surface);
}

void me::CanonicalWindowManagerPolicy::handle_surface_ready(SurfaceInfo& surface_info)
{
    select_active_surface(surface_info.surface);
}

void me::CanonicalWindowManagerPolicy::handle_modify_surface(
    SurfaceInfo& surface_info,
    shell::SurfaceSpecification const& modifications)
{
    auto surface_info_new = surface_info;
    std::shared_ptr<scene::Surface> const surface{surface_info_new.surface};

    if (modifications.parent.is_set())
        surface_info_new.parent = tools->info_for(modifications.parent.value()).surface;

    if (modifications.type.is_set() &&
        surface_info_new.type != modifications.type.value())
    {
        auto const new_type = modifications.type.value();

        if (!surface_info_new.can_morph_to(new_type))
        {
            throw std::runtime_error("Unsupported surface type change");
        }

        surface_info_new.type = new_type;

        if (surface_info_new.must_not_have_parent())
        {
            if (modifications.parent.is_set())
                throw std::runtime_error("Target surface type does not support parent");

            surface_info_new.parent.reset();
        }
        else if (surface_info_new.must_have_parent())
        {
            if (!surface_info_new.parent)
                throw std::runtime_error("Target surface type requires parent");
        }

        surface->configure(mir_surface_attrib_type, new_type);
    }

    #define COPY_IF_SET(field)\
        if (modifications.field.is_set())\
            surface_info_new.field = modifications.field.value()

    COPY_IF_SET(min_width);
    COPY_IF_SET(min_height);
    COPY_IF_SET(max_width);
    COPY_IF_SET(max_height);
    COPY_IF_SET(min_width);
    COPY_IF_SET(width_inc);
    COPY_IF_SET(height_inc);
    COPY_IF_SET(min_aspect);
    COPY_IF_SET(max_aspect);
    COPY_IF_SET(output_id);

    #undef COPY_IF_SET

    std::swap(surface_info_new, surface_info);

    if (modifications.name.is_set())
        surface->rename(modifications.name.value());

    if (modifications.streams.is_set())
    {
        auto v = modifications.streams.value();
        std::vector<shell::StreamSpecification> l (v.begin(), v.end());
        surface_info_new.surface.session()->configure_streams(*surface, l);
    }

    if (modifications.input_shape.is_set())
    {
        surface->set_input_region(modifications.input_shape.value());
    }

    if (modifications.width.is_set() || modifications.height.is_set())
    {
        auto new_size = surface->size();

        if (modifications.width.is_set())
            new_size.width = modifications.width.value();

        if (modifications.height.is_set())
            new_size.height = modifications.height.value();

        auto top_left = surface->top_left();

        surface_info.constrain_resize(top_left, new_size);

        apply_resize(surface_info, top_left, new_size);
    }

    if (modifications.state.is_set())
    {
        auto const state = handle_set_state(surface_info, modifications.state.value());
        surface->configure(mir_surface_attrib_state, state);
    }
}

void me::CanonicalWindowManagerPolicy::handle_delete_surface(SurfaceInfo& surface_info)
{
    std::shared_ptr<ms::Session> const session{surface_info.surface.session()};
    auto& surface{surface_info.surface};

    fullscreen_surfaces.erase(surface);

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

    if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(surface_info.userdata))
    {
        titlebar->surface.destroy_surface();
        tools->forget(titlebar->surface);
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

    surface.destroy_surface();

    if (surfaces.empty() && session == tools->focused_session())
    {
        active_surface_.reset();
        tools->focus_next_session();
        select_active_surface(tools->focused_surface());
    }
}

auto me::CanonicalWindowManagerPolicy::handle_set_state(SurfaceInfo& surface_info, MirSurfaceState value)
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
        return surface_info.state;
    }

    if (surface_info.state == mir_surface_state_restored)
    {
        surface_info.restore_rect = {surface_info.surface.top_left(), surface_info.surface.size()};
    }

    if (surface_info.state != mir_surface_state_fullscreen)
    {
        surface_info.output_id = decltype(surface_info.output_id){};
        fullscreen_surfaces.erase(surface_info.surface);
    }
    else
    {
        fullscreen_surfaces.insert(surface_info.surface);
    }

    if (surface_info.state == value)
    {
        return surface_info.state;
    }

    auto const old_pos = surface_info.surface.top_left();
    Displacement movement;

    switch (value)
    {
    case mir_surface_state_restored:
        movement = surface_info.restore_rect.top_left - old_pos;
        surface_info.surface.resize(surface_info.restore_rect.size);
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(surface_info.userdata))
        {
            titlebar->surface.resize(titlebar_size_for_window(surface_info.restore_rect.size));
            titlebar->surface.show();
        }
        break;

    case mir_surface_state_maximized:
        movement = display_area.top_left - old_pos;
        surface_info.surface.resize(display_area.size);
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(surface_info.userdata))
            titlebar->surface.hide();
        break;

    case mir_surface_state_horizmaximized:
        movement = Point{display_area.top_left.x, surface_info.restore_rect.top_left.y} - old_pos;
        surface_info.surface.resize({display_area.size.width, surface_info.restore_rect.size.height});
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(surface_info.userdata))
        {
            titlebar->surface.resize(titlebar_size_for_window({display_area.size.width, surface_info.restore_rect.size.height}));
            titlebar->surface.show();
        }
        break;

    case mir_surface_state_vertmaximized:
        movement = Point{surface_info.restore_rect.top_left.x, display_area.top_left.y} - old_pos;
        surface_info.surface.resize({surface_info.restore_rect.size.width, display_area.size.height});
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(surface_info.userdata))
            titlebar->surface.hide();
        break;

    case mir_surface_state_fullscreen:
    {
        Rectangle rect{old_pos, surface_info.surface.size()};

        if (surface_info.output_id.is_set())
        {
            display_layout->place_in_output(surface_info.output_id.value(), rect);
        }
        else
        {
            display_layout->size_to_output(rect);
        }

        movement = rect.top_left - old_pos;
        surface_info.surface.resize(rect.size);
        break;
    }

    case mir_surface_state_hidden:
    case mir_surface_state_minimized:
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(surface_info.userdata))
            titlebar->surface.hide();
        surface_info.surface.hide();
        return surface_info.state = value;

    default:
        break;
    }

    // TODO It is rather simplistic to move a tree WRT the top_left of the root
    // TODO when resizing. But for more sophistication we would need to encode
    // TODO some sensible layout rules.
    move_tree(surface_info, movement);

    surface_info.state = value;

    if (surface_info.is_visible())
        surface_info.surface.show();

    return surface_info.state;
}

void me::CanonicalWindowManagerPolicy::drag(Point cursor)
{
    select_active_surface(tools->surface_at(old_cursor));
    drag(active_surface(), cursor, old_cursor, display_area);
}

void me::CanonicalWindowManagerPolicy::handle_raise_surface(SurfaceInfo& surface_info)
{
    select_active_surface(surface_info.surface);
}

bool me::CanonicalWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
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
        if (auto const session = tools->focused_session())
        {
            switch (modifiers)
            {
            case mir_input_event_modifier_alt:
                kill(session.process_id(), SIGTERM);
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
        if (auto const surface = tools->focused_surface())
            select_active_surface(surface);

        return true;
    }
    else if (action == mir_keyboard_action_down &&
            modifiers == mir_input_event_modifier_alt &&
            scan_code == KEY_GRAVE)
    {
        if (auto const prev = tools->focused_surface())
        {
            if (auto const app = tools->focused_session())
                select_active_surface(app.surface_after(prev));
        }

        return true;
    }

    return false;
}

bool me::CanonicalWindowManagerPolicy::handle_touch_event(MirTouchEvent const* event)
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

bool me::CanonicalWindowManagerPolicy::handle_pointer_event(MirPointerEvent const* event)
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

        if (mir_pointer_event_button_state(event, mir_pointer_button_tertiary))
        {
            resize(cursor);
            consumes_event = true;
        }
    }
    else if (action == mir_pointer_action_motion && !modifiers)
    {
        if (mir_pointer_event_button_state(event, mir_pointer_button_primary))
        {
            // TODO this is a rather roundabout way to detect a titlebar
            if (auto const possible_titlebar = tools->surface_at(old_cursor))
            {
                if (auto const parent = tools->info_for(possible_titlebar).parent)
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

    old_cursor = cursor;
    return consumes_event;
}

void me::CanonicalWindowManagerPolicy::toggle(MirSurfaceState state)
{
    if (auto surface = active_surface())
    {
        auto& info = tools->info_for(surface);

        if (info.state == state)
            state = mir_surface_state_restored;

        auto const value = handle_set_state(info, MirSurfaceState(state));
        surface.configure(mir_surface_attrib_state, value);
    }
}

void me::CanonicalWindowManagerPolicy::select_active_surface(Surface const& surface)
{
    if (surface == active_surface_)
        return;

    if (!surface)
    {
        if (auto const active_surface = active_surface_)
        {
            auto& info = tools->info_for(active_surface);
            if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(info.userdata))
            {
                titlebar->paint_titlebar(0x3F);
            }
        }

        if (active_surface_)
            tools->set_focus_to({});

        active_surface_.reset();
        return;
    }

    auto const& info_for = tools->info_for(surface);

    if (info_for.can_be_active())
    {
        if (auto const active_surface = active_surface_)
        {
            auto& info = tools->info_for(active_surface);
            if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(info.userdata))
            {
                titlebar->paint_titlebar(0x3F);
            }
        }
        auto& info = tools->info_for(surface);
        if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(info.userdata))
        {
            titlebar->paint_titlebar(0xFF);
        }
        tools->set_focus_to(info_for.surface);
        tools->raise_tree(info_for.surface);
        active_surface_ = info_for.surface;
    }
    else
    {
        // Cannot have input focus - try the parent
        if (auto const parent = info_for.parent)
            select_active_surface(parent);
    }
}

auto me::CanonicalWindowManagerPolicy::active_surface() const
-> Surface
{
    if (auto const surface = active_surface_)
        return surface;

    if (auto const session = tools->focused_session())
    {
        if (auto const surface = session.default_surface())
            return surface;
    }

    return Surface{};
}

bool me::CanonicalWindowManagerPolicy::resize(Surface const& surface, Point cursor, Point old_cursor)
{
    if (!surface || !surface.input_area_contains(old_cursor))
        return false;

    auto const top_left = surface.top_left();
    Rectangle const old_pos{top_left, surface.size()};

    auto anchor = top_left;

    for (auto const& corner : {
        old_pos.top_right(),
        old_pos.bottom_left(),
        old_pos.bottom_right()})
    {
        if ((old_cursor - anchor).length_squared() <
            (old_cursor - corner).length_squared())
        {
            anchor = corner;
        }
    }

    bool const left_resize = anchor.x != top_left.x;
    bool const top_resize  = anchor.y != top_left.y;
    int const x_sign = left_resize? -1 : 1;
    int const y_sign = top_resize?  -1 : 1;

    auto const delta = cursor-old_cursor;

    Size new_size{old_pos.size.width + x_sign*delta.dx, old_pos.size.height + y_sign*delta.dy};

    Point new_pos = top_left + left_resize*delta.dx + top_resize*delta.dy;

    auto& surface_info = tools->info_for(surface);

    apply_resize(surface_info, new_pos, new_size);

    return true;
}

void me::CanonicalWindowManagerPolicy::apply_resize(SurfaceInfo& surface_info, Point new_pos, Size new_size) const
{
    surface_info.constrain_resize(new_pos, new_size);

    if (auto const titlebar = std::static_pointer_cast<CanonicalWindowManagementPolicyData>(surface_info.userdata))
        titlebar->surface.resize({new_size.width, Height{title_bar_height}});

    surface_info.surface.resize(new_size);

    move_tree(surface_info, new_pos-surface_info.surface.top_left());
}

bool me::CanonicalWindowManagerPolicy::drag(Surface surface, Point to, Point from, Rectangle /*bounds*/)
{
    if (!surface)
        return false;

    auto& surface_info = tools->info_for(surface);

    if (!surface.input_area_contains(from) &&
        !std::static_pointer_cast<CanonicalWindowManagementPolicyData>(surface_info.userdata))
        return false;

    auto movement = to - from;

    // placeholder - constrain onscreen

    switch (surface_info.state)
    {
    case mir_surface_state_restored:
        break;

    // "A vertically maximised surface is anchored to the top and bottom of
    // the available workspace and can have any width."
    case mir_surface_state_vertmaximized:
        movement.dy = DeltaY(0);
        break;

    // "A horizontally maximised surface is anchored to the left and right of
    // the available workspace and can have any height"
    case mir_surface_state_horizmaximized:
        movement.dx = DeltaX(0);
        break;

    // "A maximised surface is anchored to the top, bottom, left and right of the
    // available workspace. For example, if the launcher is always-visible then
    // the left-edge of the surface is anchored to the right-edge of the launcher."
    case mir_surface_state_maximized:
    case mir_surface_state_fullscreen:
    default:
        return true;
    }

    move_tree(surface_info, movement);

    return true;
}

void me::CanonicalWindowManagerPolicy::move_tree(SurfaceInfo& root, Displacement movement) const
{
    root.surface.move_to(root.surface.top_left() + movement);

    for (auto const& child: root.children)
    {
        move_tree(tools->info_for(child), movement);
    }
}
