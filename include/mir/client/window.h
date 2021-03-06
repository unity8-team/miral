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

#ifndef MIR_CLIENT_WINDOW_H
#define MIR_CLIENT_WINDOW_H

#include <mir/client/detail/mir_forward_compatibility.h>

#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
#include <mir_toolkit/mir_surface.h>
auto const mir_window_release_sync      = mir_surface_release_sync;
auto const mir_window_release           = mir_surface_release;
auto const mir_window_get_buffer_stream = mir_surface_get_buffer_stream;
auto const mir_window_is_valid          = mir_surface_is_valid;
auto const mir_window_get_error_message = mir_surface_get_error_message;
auto const mir_window_set_state         = mir_surface_set_state;
#else
#include <mir_toolkit/mir_window.h>
#endif

#include <memory>

namespace mir
{
namespace client
{
/// Handle class for MirWindow - provides automatic reference counting.
class Window
{
public:
    Window() = default;
    explicit Window(MirWindow* spec) : self{spec, deleter} {}


    operator MirWindow*() const { return self.get(); }

    void reset() { self.reset(); }

private:
    static void deleter(MirWindow* window) { mir_window_release_sync(window); }
    std::shared_ptr<MirWindow> self;
};

// Provide a deleted overload to avoid double release "accidents".
void mir_window_release_sync(Window const& window) = delete;
void mir_surface_release_sync(Window const& window) = delete;
}
}

#endif //MIR_CLIENT_WINDOW_H
