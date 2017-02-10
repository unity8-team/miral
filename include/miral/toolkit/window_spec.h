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

#ifndef MIRAL_TOOLKIT_WINDOW_SPEC_H
#define MIRAL_TOOLKIT_WINDOW_SPEC_H

#include <miral/toolkit/window.h>
#include <miral/detail/mir_forward_compatibility.h>

#include <mir_toolkit/mir_connection.h>

#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
#include <mir_toolkit/mir_surface.h>
#else
#include <mir_toolkit/mir_window.h>
#endif

#include <memory>

namespace miral
{
namespace toolkit
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
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
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
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
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
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
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
#if MIR_CLIENT_VERSION > MIR_VERSION_NUMBER(3, 4, 0)
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
#if MIR_CLIENT_VERSION > MIR_VERSION_NUMBER(3, 4, 0)
        // There's no mir_create_satellite_window_spec()
        auto spec = WindowSpec{mir_create_window_spec(connection)}
#else
        // There's no mir_create_satellite_window_spec()
        auto spec = WindowSpec{mir_create_surface_spec(connection)}
#endif
            .set_buffer_usage(mir_buffer_usage_hardware) // Required protobuf field for create_window()
            .set_pixel_format(mir_pixel_format_invalid)  // Required protobuf field for create_window()
            .set_size(width, height)
            .set_type(mir_window_type_satellite)
            .set_parent(parent);
        return spec;
    }

    static auto for_changes(MirConnection* connection) -> WindowSpec
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        return WindowSpec{mir_connection_create_spec_for_changes(connection)};
#else
        return WindowSpec{mir_create_window_spec(connection)};
#endif
    }

    auto set_buffer_usage(MirBufferUsage usage) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_buffer_usage(*this, usage);
#else
        mir_window_spec_set_buffer_usage(*this, usage);
#endif
        return *this;
    }

    auto set_pixel_format(MirPixelFormat format) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_pixel_format(*this, format);
#else
        mir_window_spec_set_pixel_format(*this, format);
#endif
        return *this;
    }

    auto set_type(MirWindowType type) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_type(*this, type);
#else
        mir_window_spec_set_type(*this, type);
#endif
        return *this;
    }

    auto set_shell_chrome(MirShellChrome chrome) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_shell_chrome(*this, chrome);
#else
        mir_window_spec_set_shell_chrome(*this, chrome);
#endif
        return *this;
    }

    auto set_min_size(int min_width, int min_height) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_min_width(*this, min_width);
        mir_surface_spec_set_min_height(*this, min_height);
#else
        mir_window_spec_set_min_width(*this, min_width);
        mir_window_spec_set_min_height(*this, min_height);
#endif
        return *this;
    }

    auto set_max_size(int max_width, int max_height) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_max_width(*this, max_width);
        mir_surface_spec_set_max_height(*this, max_height);
#else
        mir_window_spec_set_max_width(*this, max_width);
        mir_window_spec_set_max_height(*this, max_height);
#endif
        return *this;
    }

    auto set_size_inc(int width_inc, int height_inc) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_width_increment(*this, width_inc);
        mir_surface_spec_set_height_increment(*this, height_inc);
#else
        mir_window_spec_set_width_increment(*this, width_inc);
        mir_window_spec_set_height_increment(*this, height_inc);
#endif
        return *this;
    }

    auto set_size(int width, int height) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_width(*this, width);
        mir_surface_spec_set_height(*this, height);
#else
        mir_window_spec_set_width(*this, width);
        mir_window_spec_set_height(*this, height);
#endif
        return *this;
    }

    auto set_name(char const* name) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_name(*this, name);
#else
        mir_window_spec_set_name(*this, name);
#endif
        return *this;
    }

#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
    auto set_event_handler(mir_surface_event_callback callback, void* context) -> WindowSpec&
    {
        mir_surface_spec_set_event_handler(*this, callback, context);
        return *this;
    }
#else
    auto set_event_handler(MirWindowEventCallback callback, void* context) -> WindowSpec&
    {
        mir_window_spec_set_event_handler(*this, callback, context);
        return *this;
    }
#endif

#if MIR_CLIENT_VERSION >= MIR_VERSION_NUMBER(3, 4, 0)
    auto set_placement(const MirRectangle* rect,
                       MirPlacementGravity rect_gravity,
                       MirPlacementGravity surface_gravity,
                       MirPlacementHints   placement_hints,
                       int                 offset_dx,
                       int                 offset_dy) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_placement(*this, rect, rect_gravity, surface_gravity, placement_hints, offset_dx, offset_dy);
#else
        mir_window_spec_set_placement(*this, rect, rect_gravity, surface_gravity, placement_hints, offset_dx, offset_dy);
#endif
        return *this;
    }
#endif

    auto set_parent(MirWindow* parent) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_parent(*this, parent);
#else
        mir_window_spec_set_parent(*this, parent);
#endif
        return *this;
    }

    auto set_state(MirWindowState state) -> WindowSpec&
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_spec_set_state(*this, state);
#else
        mir_window_spec_set_state(*this, state);
#endif
        return *this;
    }

    template<typename Context>
    void create_window(void (* callback)(MirWindow*, Context*), Context* context) const
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_create(*this, reinterpret_cast<mir_surface_callback>(callback), context);
#else
        mir_create_window(*this, reinterpret_cast<MirWindowCallback>(callback), context);
#endif
    }

    auto create_window() const -> Window
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        return Window{mir_surface_create_sync(*this)};
#else
        return Window{mir_create_window_sync(*this)};
#endif
    }

    void apply_to(MirWindow* window) const
    {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        mir_surface_apply_spec(window, *this);
#else
        mir_window_apply_spec(window, *this);
#endif
    }

    operator MirWindowSpec*() const { return self.get(); }

private:
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
    static void deleter(MirWindowSpec* spec) { mir_surface_spec_release(spec); }
#else
    static void deleter(MirWindowSpec* spec) { mir_window_spec_release(spec); }
#endif
    std::shared_ptr<MirWindowSpec> self;
};
}
}

#endif //MIRAL_TOOLKIT_WINDOW_SPEC_H_H
