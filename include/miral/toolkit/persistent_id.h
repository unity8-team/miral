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

#ifndef MIRAL_TOOLKIT_PERSISTENT_ID_H
#define MIRAL_TOOLKIT_PERSISTENT_ID_H

#include <miral/detail/mir_forward_compatibility.h>
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
#include <mir_toolkit/mir_surface.h>
#else
#include <mir_toolkit/mir_window.h>
#endif
#include <mir_toolkit/mir_persistent_id.h>

#include <memory>

namespace miral
{
namespace toolkit
{
/// Handle class for MirPersistentId - provides automatic reference counting
class PersistentId
{
public:
    explicit PersistentId(MirPersistentId* id) : self{id, deleter} {}
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
    explicit PersistentId(MirSurface* surface) : PersistentId{mir_surface_request_persistent_id_sync(surface)} {}
#else
    explicit PersistentId(MirWindow* surface) : PersistentId{mir_window_request_persistent_id_sync(surface)} {}
#endif

    auto c_str() const -> char const* { return mir_persistent_id_as_string(self.get()); }

private:
    static void deleter(MirPersistentId* id) { mir_persistent_id_release(id); }
    std::shared_ptr<MirPersistentId> self;
};
}
}

#endif //MIRAL_TOOLKIT_PERSISTENT_ID_H
