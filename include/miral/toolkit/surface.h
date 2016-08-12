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

#ifndef MIRAL_TOOLKIT_SURFACE_H
#define MIRAL_TOOLKIT_SURFACE_H

#include <mir_toolkit/mir_surface.h>

#include <memory>

namespace miral
{
namespace toolkit
{
/// Handle class for MirSurface - provides automatic reference counting, method chaining.
class Surface
{
public:
    Surface() = default;
    explicit Surface(MirSurface* spec) : self{spec, deleter} {}


    operator MirSurface*() const { return self.get(); }

private:
    static void deleter(MirSurface* surface) { mir_surface_release_sync(surface); }
    std::shared_ptr<MirSurface> self;
};
}
}

#endif //MIRAL_TOOLKIT_SURFACE_H
