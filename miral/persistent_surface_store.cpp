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

#include "persistent_surface_store.h"

#include <mir/shell/persistent_surface_store.h>
#include <mir/server.h>
#include <mir/version.h>

miral::PersistentSurfaceStore::PersistentSurfaceStore() = default;
miral::PersistentSurfaceStore::~PersistentSurfaceStore() = default;
miral::PersistentSurfaceStore::PersistentSurfaceStore(PersistentSurfaceStore const&) = default;
miral::PersistentSurfaceStore& miral::PersistentSurfaceStore::operator=(PersistentSurfaceStore const&) = default;

void miral::PersistentSurfaceStore::operator()(mir::Server& server)
{
#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 24, 0)
    server.add_init_callback[this] { self = server.the_persistent_surface_store(); });
#else
    (void)server;
#endif
}

auto miral::PersistentSurfaceStore::id_for_surface(std::shared_ptr<mir::scene::Surface> const& surface) -> Id
{
    if (auto myself = self.lock())
        return myself->id_for_surface(surface).serialize_to_string();
    else
        throw std::logic_error{"PersistentSurfaceStore not initialized"};
}

auto miral::PersistentSurfaceStore::surface_for_id(Id const& id) const -> std::shared_ptr<mir::scene::Surface>
{
    if (auto myself = self.lock())
        return myself->surface_for_id(id);
    else
        return {};
}
