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

#include "miral/internal_client.h"

#include <mir_toolkit/mir_connection.h>
#include <mir/fd.h>
#include <mir/server.h>
#include <mir/scene/session.h>
#include <mir/main_loop.h>

#include <condition_variable>
#include <mutex>
#include <thread>

class miral::InternalClient::Self
{
public:
    Self(std::string name,
         std::function<void(MirConnection* connection)> client_code,
         std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification);

    void run(mir::Server& server);
    ~Self();

private:
    std::string const name;
    std::thread thread;
    std::mutex mutable mutex;
    std::condition_variable mutable cv;
    mir::Fd fd;
    std::weak_ptr<mir::scene::Session> session;
    MirConnection* connection = nullptr;
    std::function<void(MirConnection* connection)> const client_code;
    std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification;
};

miral::InternalClient::Self::Self(
    std::string const name,
    std::function<void(MirConnection* connection)> client_code,
    std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification) :
    name(name),
    client_code(std::move(client_code)),
    connect_notification(std::move(connect_notification))
{
}

void miral::InternalClient::Self::run(mir::Server& server)
{
    fd = server.open_client_socket([this](std::shared_ptr<mir::frontend::Session> const& mf_session)
        {
            std::lock_guard<decltype(mutex)> lock_guard{mutex};
            session = std::dynamic_pointer_cast<mir::scene::Session>(mf_session);
            cv.notify_one();
            connect_notification(session);
        });

    char connect_string[64] = {0};
    sprintf(connect_string, "fd://%d", fd.operator int());

    connection = mir_connect_sync(connect_string, name.c_str());

    std::unique_lock<decltype(mutex)> lock{mutex};
    cv.wait(lock, [&] { return !!session.lock(); });

    thread = std::thread{[this] { client_code(connection); }};
}

miral::InternalClient::Self::~Self()
{
    if (thread.joinable())
    {
        thread.join();
        mir_connection_release(connection);
    }
}

miral::InternalClient::InternalClient(
    std::string name,
    std::function<void(MirConnection* connection)> client_code,
    std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification) :
    internal_client(std::make_shared<Self>(std::move(name), std::move(client_code), std::move(connect_notification)))
{
}

void miral::InternalClient::operator()(mir::Server& server)
{
    server.add_init_callback([this, &server]
    {
        server.the_main_loop()->enqueue(this, [this, &server]
        {
            internal_client->run(server);
        });
    });
}

miral::InternalClient::~InternalClient() = default;
