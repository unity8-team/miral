/*
 * Copyright © 2016-2017 Canonical Ltd.
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

#ifndef MIR_CLIENT_WINDOW_SPEC_H
#define MIR_CLIENT_WINDOW_SPEC_H

#include <mir/client/window.h>
#include <mir/client/detail/mir_forward_compatibility.h>

#include <mir_toolkit/mir_connection.h>

#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
#include <mir_toolkit/mir_surface.h>
#else
#include <mir_toolkit/mir_window.h>
#endif

#include <memory>

// Forward compatibility hacks for earlier Mir versions
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
using MirWindowCallback = mir_surface_callback;
using MirWindowEventCallback = mir_surface_event_callback;
auto const mir_create_window_spec               = mir_connection_create_spec_for_changes;
auto const mir_window_spec_set_event_handler    = mir_surface_spec_set_event_handler;
auto const mir_window_spec_set_name             = mir_surface_spec_set_name;
auto const mir_window_spec_set_width            = mir_surface_spec_set_width;
auto const mir_window_spec_set_height           = mir_surface_spec_set_height;
auto const mir_window_spec_set_width_increment  = mir_surface_spec_set_width_increment;
auto const mir_window_spec_set_height_increment = mir_surface_spec_set_height_increment;
auto const mir_window_spec_set_buffer_usage     = mir_surface_spec_set_buffer_usage;
auto const mir_window_spec_set_pixel_format     = mir_surface_spec_set_pixel_format;
auto const mir_window_spec_set_type             = mir_surface_spec_set_type;
auto const mir_window_spec_set_shell_chrome     = mir_surface_spec_set_shell_chrome;
auto const mir_window_spec_set_min_width        = mir_surface_spec_set_min_width;
auto const mir_window_spec_set_min_height       = mir_surface_spec_set_min_height;
auto const mir_window_spec_set_max_width        = mir_surface_spec_set_max_width;
auto const mir_window_spec_set_max_height       = mir_surface_spec_set_max_height;
auto const mir_window_spec_set_parent           = mir_surface_spec_set_parent;
auto const mir_window_spec_set_state            = mir_surface_spec_set_state;
auto const mir_window_spec_set_fullscreen_on_output = mir_surface_spec_set_fullscreen_on_output;
auto const mir_create_window                    = mir_surface_create;
auto const mir_create_window_sync               = mir_surface_create_sync;
auto const mir_window_apply_spec                = mir_surface_apply_spec;
auto const mir_window_spec_release              = mir_surface_spec_release;

#if MIR_CLIENT_VERSION >= MIR_VERSION_NUMBER(3, 4, 0)
auto const mir_window_spec_set_placement        = mir_surface_spec_set_placement;
#endif
#endif

namespace mir
{
namespace client
{
/// Handle class for MirWindowSpec - provides automatic reference counting, method chaining.
class WindowSpec
{
public:
    explicit WindowSpec(MirWindowSpec* spec) : self{spec, deleter} {}

    static auto for_normal_window(MirConnection* connection, int width, int height, MirPixelFormat format) -> WindowSpec
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        return WindowSpec{mir_connection_create_spec_for_normal_surface(connection, width, height, format)};
#else
        auto spec = WindowSpec{mir_create_normal_window_spec(connection, width, height)};
        mir_window_spec_set_pixel_format(spec, format);
        return spec;
#endif
    }

#if MIR_CLIENT_VERSION > MIR_VERSION_NUMBER(3, 4, 0)
    static auto for_normal_window(MirConnection* connection, int width, int height) -> WindowSpec
    {
        return WindowSpec{mir_create_normal_window_spec(connection, width, height)};
    }
#endif

    static auto for_menu(MirConnection* connection,
                         int width,
                         int height,
                         MirPixelFormat format,
                         MirWindow* parent,
                         MirRectangle* rect,
                         MirEdgeAttachment edge) -> WindowSpec
    {
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
        return WindowSpec{mir_connection_create_spec_for_menu(connection, width, height, format, parent, rect, edge)};
#else
        auto spec = WindowSpec{mir_create_menu_window_spec(connection, width, height, parent, rect, edge)};
        mir_window_spec_set_pixel_format(spec, format);
        return spec;
#endif
    }

#if MIR_CLIENT_VERSION >= MIR_VERSION_NUMBER(3, 4, 0)
    static auto for_tip(MirConnection* connection,
                        int width,
                        int height,
                        MirPixelFormat format,
                        MirWindow* parent,
                        MirRectangle* rect,
                        MirEdgeAttachment edge) -> WindowSpec
    {
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
        return WindowSpec{mir_connection_create_spec_for_tip(connection, width, height, format, parent, rect, edge)};
#else
        auto spec = WindowSpec{mir_create_tip_window_spec(connection, width, height, parent, rect, edge)};
        mir_window_spec_set_pixel_format(spec, format);
        return spec;
#endif
    }
#endif

    static auto for_dialog(MirConnection* connection,
                           int width,
                           int height,
                           MirPixelFormat format)-> WindowSpec
    {
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
        return WindowSpec{mir_connection_create_spec_for_dialog(connection, width, height, format)};
#else
        auto spec = WindowSpec{mir_create_dialog_window_spec(connection, width, height)};
        mir_window_spec_set_pixel_format(spec, format);
        return spec;
#endif
    }

    static auto for_dialog(MirConnection* connection,
                           int width,
                           int height,
                           MirPixelFormat format,
                           MirWindow* parent) -> WindowSpec
    {
        return for_dialog(connection, width, height, format).set_parent(parent);
    }

    static auto for_input_method(MirConnection* connection, int width, int height, MirWindow* parent)
    {
#if MIR_CLIENT_VERSION >= MIR_VERSION_NUMBER(3, 5, 0)
        auto spec = WindowSpec{mir_create_input_method_window_spec(connection, width, height)}
#else
        auto spec = WindowSpec{mir_create_surface_spec(connection)}
            .set_buffer_usage(mir_buffer_usage_hardware) // Required protobuf field for create_window()
            .set_pixel_format(mir_pixel_format_invalid)  // Required protobuf field for create_window()
            .set_size(width, height)
            .set_type(mir_window_type_inputmethod)
#endif
            .set_parent(parent);
        return spec;
    }

    static auto for_satellite(MirConnection* connection, int width, int height, MirWindow* parent)
    {
        // There's no mir_create_satellite_window_spec()
        return WindowSpec{mir_create_window_spec(connection)}
            .set_buffer_usage(mir_buffer_usage_hardware) // Required protobuf field for create_window()
            .set_pixel_format(mir_pixel_format_invalid)  // Required protobuf field for create_window()
            .set_size(width, height)
            .set_type(mir_window_type_satellite)
            .set_parent(parent);
    }

    static auto for_gloss(MirConnection* connection, int width, int height)
    {
        // There's no mir_create_gloss_window_spec()
        return WindowSpec{mir_create_window_spec(connection)}
            .set_buffer_usage(mir_buffer_usage_hardware) // Required protobuf field for create_window()
            .set_pixel_format(mir_pixel_format_invalid)  // Required protobuf field for create_window()
            .set_size(width, height)
            .set_type(mir_window_type_gloss);
    }

    static auto for_changes(MirConnection* connection) -> WindowSpec
    {
        return WindowSpec{mir_create_window_spec(connection)};
    }

    auto set_buffer_usage(MirBufferUsage usage) -> WindowSpec&
    {
        mir_window_spec_set_buffer_usage(*this, usage);
        return *this;
    }

    auto set_pixel_format(MirPixelFormat format) -> WindowSpec&
    {
        mir_window_spec_set_pixel_format(*this, format);
        return *this;
    }

    auto set_type(MirWindowType type) -> WindowSpec&
    {
        mir_window_spec_set_type(*this, type);
        return *this;
    }

    auto set_shell_chrome(MirShellChrome chrome) -> WindowSpec&
    {
        mir_window_spec_set_shell_chrome(*this, chrome);
        return *this;
    }

    auto set_min_size(int min_width, int min_height) -> WindowSpec&
    {
        mir_window_spec_set_min_width(*this, min_width);
        mir_window_spec_set_min_height(*this, min_height);
        return *this;
    }

    auto set_max_size(int max_width, int max_height) -> WindowSpec&
    {
        mir_window_spec_set_max_width(*this, max_width);
        mir_window_spec_set_max_height(*this, max_height);
        return *this;
    }

    auto set_size_inc(int width_inc, int height_inc) -> WindowSpec&
    {
        mir_window_spec_set_width_increment(*this, width_inc);
        mir_window_spec_set_height_increment(*this, height_inc);
        return *this;
    }

    auto set_size(int width, int height) -> WindowSpec&
    {
        mir_window_spec_set_width(*this, width);
        mir_window_spec_set_height(*this, height);
        return *this;
    }

    auto set_name(char const* name) -> WindowSpec&
    {
        mir_window_spec_set_name(*this, name);
        return *this;
    }

    auto set_event_handler(MirWindowEventCallback callback, void* context) -> WindowSpec&
    {
        mir_window_spec_set_event_handler(*this, callback, context);
        return *this;
    }

    auto set_fullscreen_on_output(uint32_t output_id) -> WindowSpec&
    {
        mir_window_spec_set_fullscreen_on_output(*this, output_id);
        return *this;
    }

#if MIR_CLIENT_VERSION >= MIR_VERSION_NUMBER(3, 4, 0)
    auto set_placement(const MirRectangle* rect,
                       MirPlacementGravity rect_gravity,
                       MirPlacementGravity surface_gravity,
                       MirPlacementHints   placement_hints,
                       int                 offset_dx,
                       int                 offset_dy) -> WindowSpec&
    {
        mir_window_spec_set_placement(*this, rect, rect_gravity, surface_gravity, placement_hints, offset_dx, offset_dy);
        return *this;
    }
#else
    auto set_placement(const MirRectangle* /*rect*/,
                       MirPlacementGravity /*rect_gravity*/,
                       MirPlacementGravity /*surface_gravity*/,
                       MirPlacementHints   /*placement_hints*/,
                       int                 /*offset_dx*/,
                       int                 /*offset_dy*/) -> WindowSpec&
    {
        return *this;
    }
#endif

    auto set_parent(MirWindow* parent) -> WindowSpec&
    {
        mir_window_spec_set_parent(*this, parent);
        return *this;
    }

    auto set_state(MirWindowState state) -> WindowSpec&
    {
        mir_window_spec_set_state(*this, state);
        return *this;
    }

    template<typename Context>
    void create_window(void (* callback)(MirWindow*, Context*), Context* context) const
    {
        mir_create_window(*this, reinterpret_cast<MirWindowCallback>(callback), context);
    }

    auto create_window() const -> Window
    {
        return Window{mir_create_window_sync(*this)};
    }

    void apply_to(MirWindow* window) const
    {
        mir_window_apply_spec(window, *this);
    }

    operator MirWindowSpec*() const { return self.get(); }

private:
    static void deleter(MirWindowSpec* spec) { mir_window_spec_release(spec); }
    std::shared_ptr<MirWindowSpec> self;
};
}
}

#endif //MIRAL_TOOLKIT_WINDOW_SPEC_H_H
