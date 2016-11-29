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

#ifndef MIRAL_TOOLKIT_SURFACE_SPEC_H
#define MIRAL_TOOLKIT_SURFACE_SPEC_H

#include <miral/toolkit/surface.h>

#include <mir_toolkit/mir_surface.h>
#include <mir_toolkit/mir_connection.h>
#include <mir_toolkit/version.h>

#include <memory>

namespace miral
{
namespace toolkit
{
/// Handle class for MirSurfaceSpec - provides automatic reference counting, method chaining.
class SurfaceSpec
{
public:
    explicit SurfaceSpec(MirSurfaceSpec* spec) : self{spec, deleter} {}

    static auto for_normal_surface(MirConnection* connection, int width, int height, MirPixelFormat format) -> SurfaceSpec
    {
        return SurfaceSpec{mir_connection_create_spec_for_normal_surface(connection, width, height, format)};
    }

    static auto for_menu(MirConnection* connection,
                         int width,
                         int height,
                         MirPixelFormat format,
                         MirSurface* parent,
                         MirRectangle* rect,
                         MirEdgeAttachment edge) -> SurfaceSpec
    {
        return SurfaceSpec{mir_connection_create_spec_for_menu(connection, width, height, format, parent, rect, edge)};
    }

    static auto for_tip(MirConnection* connection,
                         int width,
                         int height,
                         MirPixelFormat format,
                         MirSurface* parent,
                         MirRectangle* rect,
                         MirEdgeAttachment edge) -> SurfaceSpec
    {
        return SurfaceSpec{mir_connection_create_spec_for_tip(connection, width, height, format, parent, rect, edge)};
    }

    static auto for_dialog(MirConnection* connection,
                           int width,
                           int height,
                           MirPixelFormat format)-> SurfaceSpec
    {
        return SurfaceSpec{mir_connection_create_spec_for_dialog(connection, width, height, format)};
    }

    static auto for_dialog(MirConnection* connection,
                        int width,
                        int height,
                        MirPixelFormat format,
                        MirSurface* parent) -> SurfaceSpec
    {
        return for_dialog(connection, width, height, format).set_parent(parent);
    }

    static auto for_changes(MirConnection* connection) -> SurfaceSpec
    {
        return SurfaceSpec{mir_connection_create_spec_for_changes(connection)};
    }

    auto set_buffer_usage(MirBufferUsage usage) -> SurfaceSpec&
    {
        mir_surface_spec_set_buffer_usage(*this, usage);
        return *this;
    }

    auto set_type(MirSurfaceType type) -> SurfaceSpec&
    {
        mir_surface_spec_set_type(*this, type);
        return *this;
    }

    auto set_min_size(int min_width, int min_height) -> SurfaceSpec&
    {
        mir_surface_spec_set_min_width(*this, min_width);
        mir_surface_spec_set_min_height(*this, min_height);
        return *this;
    }

    auto set_max_size(int max_width, int max_height) -> SurfaceSpec&
    {
        mir_surface_spec_set_max_width(*this, max_width);
        mir_surface_spec_set_max_height(*this, max_height);
        return *this;
    }

    auto set_size_inc(int width_inc, int height_inc) -> SurfaceSpec&
    {
        mir_surface_spec_set_width_increment(*this, width_inc);
        mir_surface_spec_set_height_increment(*this, height_inc);
        return *this;
    }

    auto set_size(int width, int height) -> SurfaceSpec&
    {
        mir_surface_spec_set_width(*this, width);
        mir_surface_spec_set_height(*this, height);
        return *this;
    }

    auto set_name(char const* name) -> SurfaceSpec&
    {
        mir_surface_spec_set_name(*this, name);
        return *this;
    }

    auto set_event_handler(mir_surface_event_callback callback, void* context) -> SurfaceSpec&
    {
        mir_surface_spec_set_event_handler(*this, callback, context);
        return *this;
    }

#if MIR_CLIENT_VERSION >= MIR_VERSION_NUMBER(3, 4, 0)
    auto set_placement(const MirRectangle* rect,
                       MirPlacementGravity rect_gravity,
                       MirPlacementGravity surface_gravity,
                       MirPlacementHints   placement_hints,
                       int                 offset_dx,
                       int                 offset_dy) -> SurfaceSpec&
    {
        mir_surface_spec_set_placement(*this, rect, rect_gravity, surface_gravity, placement_hints, offset_dx, offset_dy);
        return *this;
    }
#endif

    auto set_parent(MirSurface* parent) -> SurfaceSpec&
    {
        mir_surface_spec_set_parent(*this, parent);
        return *this;
    }

    template<typename Context>
    void create_surface(void (*callback)(MirSurface*, Context*), Context* context) const
    {
        mir_surface_create(*this, reinterpret_cast<mir_surface_callback>(callback), context);
    }

    auto create_surface() const -> Surface
    {
        return Surface{mir_surface_create_sync(*this)};
    }

    void apply_to(MirSurface* surface) const
    {
        mir_surface_apply_spec(surface, *this);
    }

    operator MirSurfaceSpec*() const { return self.get(); }

private:
    static void deleter(MirSurfaceSpec* spec) { mir_surface_spec_release(spec); }
    std::shared_ptr<MirSurfaceSpec> self;
};
}
}

#endif //MIRAL_TOOLKIT_SURFACE_SPEC_H_H
