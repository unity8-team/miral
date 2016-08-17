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

#include "test_server.h"

#include <miral/canonical_window_manager.h>
#include <miral/set_window_managment_policy.h>

#include <mir_test_framework/executable_path.h>
#include "mir_test_framework/stub_server_platform_factory.h"

#include <mir/fd.h>
#include <mir/main_loop.h>
#include <mir/server.h>
#include <mir/version.h>

#include <boost/throw_exception.hpp>


using namespace miral;
using namespace testing;
namespace mtf = mir_test_framework;

namespace
{
std::chrono::seconds const timeout{20};
char const* dummy_args[2] = { "TestServer", nullptr };
}

miral::TestServer::TestServer() :
    runner{1, dummy_args}
{
    add_to_environment("MIR_SERVER_PLATFORM_GRAPHICS_LIB", mtf::server_platform("graphics-dummy.so").c_str());
    add_to_environment("MIR_SERVER_PLATFORM_INPUT_LIB", mtf::server_platform("input-stub.so").c_str());
}

void miral::TestServer::SetUp()
{
#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 25, 0)
    mtf::set_next_preset_display({}); // Workaround for lp:1611337
#endif

    mir::test::AutoJoinThread t([this]
         {
            auto init = [this](mir::Server& server)
                {
                    server.add_init_callback([&]
                        {
                            auto const main_loop = server.the_main_loop();
                            // By enqueuing the notification code in the main loop, we are
                            // ensuring that the server has really and fully started before
                            // leaving start_mir_server().
                            main_loop->enqueue(this, [&]
                                {
                                     std::lock_guard<std::mutex> lock(mutex);
                                     server_running = &server;
                                     started.notify_one();
                                });
                        });
                };

            try
            {
                runner.run_with({init, set_window_managment_policy<TestWindowManagerPolicy>(*this)});
            }
            catch (std::exception const& e)
            {
                FAIL() << e.what();
            }

            std::lock_guard<std::mutex> lock(mutex);
            server_running = nullptr;
            started.notify_one();
         });

    std::unique_lock<std::mutex> lock(mutex);
    started.wait_for(lock, timeout, [&] { return server_running; });

    if (!server_running)
        BOOST_THROW_EXCEPTION(std::runtime_error{"Failed to start server thread"});

    server_thread = std::move(t);
}

void miral::TestServer::TearDown()
{
    std::unique_lock<std::mutex> lock(mutex);

    runner.stop();

    started.wait_for(lock, timeout, [&] { return !server_running; });

    if (server_running)
        BOOST_THROW_EXCEPTION(std::logic_error{"Failed to stop server"});

    server_thread.stop();
}

auto miral::TestServer::connect_client(std::string name) -> toolkit::Connection
{
    std::lock_guard<std::mutex> lock(mutex);

    if (!server_running)
        BOOST_THROW_EXCEPTION(std::runtime_error{"Server not running"});

    char connect_string[64] = {0};
    sprintf(connect_string, "fd://%d", dup(server_running->open_client_socket()));

    return toolkit::Connection{mir_connect_sync(connect_string, name.c_str())};
}

void miral::TestRuntimeEnvironment::add_to_environment(char const* key, char const* value)
{
    env.emplace_back(key, value);
}

struct miral::TestServer::TestWindowManagerPolicy : CanonicalWindowManagerPolicy
{
    TestWindowManagerPolicy(WindowManagerTools const& tools, TestServer& test_fixture) :
        CanonicalWindowManagerPolicy{tools}
    {
        test_fixture.tools = tools;
    }

    bool handle_keyboard_event(MirKeyboardEvent const*) override { return false; }
    bool handle_pointer_event(MirPointerEvent const*) override { return false; }
    bool handle_touch_event(MirTouchEvent const*) override { return false; }
};

using miral::TestServer;

// Minimal test to ensure the server runs and exits
TEST_F(TestServer, connect_client_works)
{
    auto const connection = connect_client(__PRETTY_FUNCTION__);

    EXPECT_TRUE(mir_connection_is_valid(connection));
}
