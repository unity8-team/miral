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

#ifndef MIRAL_TEST_SERVER_H
#define MIRAL_TEST_SERVER_H

#include <miral/toolkit/connection.h>

#include <miral/runner.h>
#include <miral/window_manager_tools.h>

#include <mir/test/auto_unblock_thread.h>
#include <mir_test_framework/temporary_environment_value.h>

#include <gtest/gtest.h>

#include <condition_variable>
#include <list>
#include <mutex>

namespace miral
{
class TestRuntimeEnvironment
{
public:
    void add_to_environment(char const* key, char const* value);

private:
    std::list<mir_test_framework::TemporaryEnvironmentValue> env;
};

struct TestServer : testing::Test, private TestRuntimeEnvironment
{
    TestServer();

    void SetUp() override;
    void TearDown() override;

    auto connect_client(std::string name) -> toolkit::Connection;

    using TestRuntimeEnvironment::add_to_environment;
    WindowManagerTools tools{nullptr};

    MirRunner runner;

private:
    struct TestWindowManagerPolicy;

    mir::test::AutoJoinThread server_thread;
    std::mutex mutex;
    std::condition_variable started;
    mir::Server* server_running{nullptr};
};
}

#endif //MIRAL_TEST_SERVER_H
