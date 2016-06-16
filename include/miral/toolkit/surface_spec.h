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

#include <mir_toolkit/mir_surface.h>
#include <mir_toolkit/mir_connection.h>

#include <memory>

namespace miral
{
namespace toolkit
{
class SurfaceSpec
{
public:
    explicit SurfaceSpec(MirSurfaceSpec* spec) : self{spec, deleter} {}

    static auto for_normal_surface(MirConnection* connection, int width, int height, MirPixelFormat format) -> SurfaceSpec
    {
        return SurfaceSpec{mir_connection_create_spec_for_normal_surface(connection, width, height, format)};
    }

    static auto for_changes(MirConnection* connection) -> SurfaceSpec
    {
        return SurfaceSpec{mir_connection_create_spec_for_changes(connection)};
    }

    void set_buffer_usage(MirBufferUsage usage)
    {
        mir_surface_spec_set_buffer_usage(*this, usage);
    }

    void set_type(MirSurfaceType type)
    {
        mir_surface_spec_set_type(*this, type);
    }

    void set_size(int width, int height)
    {
        mir_surface_spec_set_width(*this, width);
        mir_surface_spec_set_height(*this, height);
    }

    template<typename Context>
    void create_surface(void (*callback)(MirSurface*, Context*), Context* context) const
    {
        mir_surface_create(*this, reinterpret_cast<mir_surface_callback>(callback), context);
    }

    operator MirSurfaceSpec*() const { return self.get(); }

private:
    static void deleter(MirSurfaceSpec* spec) { mir_surface_spec_release(spec); }
    std::shared_ptr<MirSurfaceSpec> self;
};
}
}

#endif //MIRAL_TOOLKIT_SURFACE_SPEC_H_H
