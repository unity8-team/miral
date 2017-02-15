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
#include "both_versions.h"

#include <mir/fd.h>
#include <mir/server.h>
#include <mir/scene/session.h>
#include <mir/main_loop.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace
{
class InternalClientRunner
{
public:
    InternalClientRunner(std::string name,
         std::function<void(mir::client::Connection connection)> client_code,
         std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification);

    void run(mir::Server& server);
    ~InternalClientRunner();

private:
    std::string const name;
    std::thread thread;
    std::mutex mutable mutex;
    std::condition_variable mutable cv;
    mir::Fd fd;
    std::weak_ptr<mir::scene::Session> session;
    mir::client::Connection connection;
    std::function<void(mir::client::Connection connection)> const client_code;
    std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification;
};
}

class miral::StartupInternalClient::Self : InternalClientRunner
{
public:
    using InternalClientRunner::InternalClientRunner;
    using InternalClientRunner::run;
};

InternalClientRunner::InternalClientRunner(
    std::string const name,
    std::function<void(mir::client::Connection connection)> client_code,
    std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification) :
    name(name),
    client_code(std::move(client_code)),
    connect_notification(std::move(connect_notification))
{
}

void InternalClientRunner::run(mir::Server& server)
{
    fd = server.open_client_socket([this](std::shared_ptr<mir::frontend::Session> const& mf_session)
        {
            std::lock_guard<decltype(mutex)> lock_guard{mutex};
            session = std::dynamic_pointer_cast<mir::scene::Session>(mf_session);
            connect_notification(session);
            cv.notify_one();
        });

    char connect_string[64] = {0};
    sprintf(connect_string, "fd://%d", fd.operator int());

    connection = mir::client::Connection{mir_connect_sync(connect_string, name.c_str())};

    std::unique_lock<decltype(mutex)> lock{mutex};
    cv.wait(lock, [&] { return !!session.lock(); });

    thread = std::thread{[this]
        {
            client_code(connection);
            connection.reset();
        }};
}

InternalClientRunner::~InternalClientRunner()
{
    if (thread.joinable())
    {
        thread.join();
    }
}

#ifndef __clang__
MIRAL_FAKE_OLD_SYMBOL(
    _ZN5miral21StartupInternalClientC1ENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESt8functionIFvNS_7toolkit10ConnectionEEES7_IFvSt8weak_ptrIN3mir5scene7SessionEEEE,
    _ZN5miral21StartupInternalClientC1ENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESt8functionIFvN3mir6client10ConnectionEEES7_IFvSt8weak_ptrINS8_5scene7SessionEEEE)
#else
MIRAL_FAKE_OLD_SYMBOL(
    _ZN5miral21StartupInternalClientC1ENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESt8functionIFvNS_7toolkit10ConnectionEEES7_IFvSt8weak_ptrIN3mir5scene7SessionEEEE,
    _ZN5miral21StartupInternalClientC2ENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESt8functionIFvN3mir6client10ConnectionEEES7_IFvSt8weak_ptrINS8_5scene7SessionEEEE)
// clang emits a different symbol ---^
#endif
miral::StartupInternalClient::StartupInternalClient(
    std::string name,
    std::function<void(mir::client::Connection connection)> client_code,
    std::function<void(std::weak_ptr<mir::scene::Session> const session)> connect_notification) :
    internal_client(std::make_shared<Self>(std::move(name), std::move(client_code), std::move(connect_notification)))
{
}

void miral::StartupInternalClient::operator()(mir::Server& server)
{
    server.add_init_callback([this, &server]
    {
        server.the_main_loop()->enqueue(this, [this, &server]
        {
            internal_client->run(server);
        });
    });
}

miral::StartupInternalClient::~StartupInternalClient() = default;

struct miral::InternalClientLauncher::Self
{
    mir::Server* server = nullptr;
    std::unique_ptr<InternalClientRunner> runner;
};

void miral::InternalClientLauncher::operator()(mir::Server& server)
{
    self->server = &server;
}


MIRAL_FAKE_OLD_SYMBOL(
    _ZNK5miral22InternalClientLauncher6launchERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEERKSt8functionIFvNS_7toolkit10ConnectionEEERKS9_IFvSt8weak_ptrIN3mir5scene7SessionEEEE,
    _ZNK5miral22InternalClientLauncher6launchERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEERKSt8functionIFvN3mir6client10ConnectionEEERKS9_IFvSt8weak_ptrINSA_5scene7SessionEEEE)
void miral::InternalClientLauncher::launch(
    std::string const& name,
    std::function<void(mir::client::Connection connection)> const& client_code,
    std::function<void(std::weak_ptr<mir::scene::Session> const session)> const& connect_notification) const
{
    self->runner = std::make_unique<InternalClientRunner>(name, client_code, connect_notification);
    self->server->the_main_loop()->enqueue(this, [this] { self->runner->run(*self->server); });
}

miral::InternalClientLauncher::InternalClientLauncher() : self{std::make_shared<Self>()} {}
miral::InternalClientLauncher::~InternalClientLauncher() = default;
