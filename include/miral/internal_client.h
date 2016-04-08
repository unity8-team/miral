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

#ifndef MIRAL_INTERNAL_CLIENT_H
#define MIRAL_INTERNAL_CLIENT_H

#include <mir_toolkit/client_types.h>

#include <functional>
#include <memory>
#include <string>

namespace mir { class Server; namespace scene { class Session; }}

namespace miral
{
/** Wrapper for running an internal Mir client session
 *  \note client_code will be executed on its own thread, this must exit
 *  \note connection_notification will be called on a worker thread and must not block
 *
 *  \param client_code              code implementing the internal client session
 *  \param connection_notification  handler for registering the server-side session
 */
class InternalClient
{
public:
    explicit InternalClient(
        std::string name,
        std::function<void(MirConnection* connection)> client_code,
        std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification);
    ~InternalClient();

    void operator()(mir::Server& server);

private:
    class Self;
    std::shared_ptr<Self> internal_client;
};
}

#endif //MIRAL_INTERNAL_CLIENT_H
