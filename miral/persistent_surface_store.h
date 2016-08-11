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

#ifndef MIRAL_PERSISTENT_SURFACE_STORE_H
#define MIRAL_PERSISTENT_SURFACE_STORE_H

#include <memory>
#include <string>

namespace mir
{
class Server;
namespace scene { class Surface; }
namespace shell { class PersistentSurfaceStore; }
}

namespace miral
{
class PersistentSurfaceStore
{
public:
    PersistentSurfaceStore();
    ~PersistentSurfaceStore();
    PersistentSurfaceStore(PersistentSurfaceStore const&);
    PersistentSurfaceStore& operator=(PersistentSurfaceStore const&);

    void operator()(mir::Server& server);

    using Id = std::string;

    /**
     * \brief Acquire ID for a Surface
     * \param [in] surface Surface to query or generate an ID for
     * \return             The ID for this surface.
     * \note If \arg surface has not yet had an ID generated, this generates its ID.
     * \note This does not extend the lifetime of \arg surface.
     * \warning Prior to Mir-0.24 the underlying APIs are not available: we throw logic_error.
     */
    auto id_for_surface(std::shared_ptr<mir::scene::Surface> const& surface) -> Id;

    /**
     * \brief Lookup Surface by ID.
     * \param [in] id    ID of surface to lookup
     * \return           The surface with ID \arg id. If this surface has been destroyed,
     *                   but the store retains a reference, returns nullptr.
     * \throws std::out_of_range if the store has no reference for a surface with \arg id.
     * \warning Prior to Mir-0.24 the underlying APIs are not available: we return nullptr.
     */
    auto surface_for_id(Id const& id) const -> std::shared_ptr<mir::scene::Surface>;

private:
    std::weak_ptr<mir::shell::PersistentSurfaceStore> self;
};
}

#endif //MIRAL_PERSISTENT_SURFACE_STORE_H
