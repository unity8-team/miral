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

#ifndef MIRAL_TOOLKIT_WINDOW_ID_H
#define MIRAL_TOOLKIT_WINDOW_ID_H

#include <miral/toolkit/detail/mir_forward_compatibility.h>
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
#include <mir_toolkit/mir_surface.h>
#else
#include <mir_toolkit/mir_window.h>
#include <mir_toolkit/mir_persistent_id.h>
#endif

#if MIR_CLIENT_API_VERSION < MIR_VERSION_NUMBER(0, 27, 0)
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
auto const mir_window_request_window_id_sync = mir_surface_request_persistent_id_sync;
#else
auto const mir_window_request_window_id_sync = mir_window_request_persistent_id_sync;
#endif
auto const mir_window_id_as_string  = mir_persistent_id_as_string;
auto const mir_window_id_release    = mir_persistent_id_release;
typedef struct MirPersistentId MirWindowId;
#else
#include <mir_toolkit/mir_window_id.h>
#endif

#include <memory>

namespace miral
{
namespace client
{
/// Handle class for MirWindowId - provides automatic reference counting
class WindowId
{
public:
    explicit WindowId(MirWindowId* id) : self{id, deleter} {}

    explicit WindowId(MirWindow* window) : WindowId{mir_window_request_window_id_sync(window)} {}

    auto c_str() const -> char const* { return mir_window_id_as_string(self.get()); }

private:
    static void deleter(MirWindowId* id) { mir_window_id_release(id); }
    std::shared_ptr<MirWindowId> self;
};
}
}

#endif //MIRAL_TOOLKIT_WINDOW_ID_H
