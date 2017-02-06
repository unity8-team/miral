/*
 * Copyright © 2017 Canonical Ltd.
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

#include <miral/workspace_policy.h>
#include <miral/window_manager_tools.h>

#include <miral/toolkit/window.h>
#include <miral/toolkit/window_spec.h>
#include <mir_toolkit/mir_buffer_stream.h>

#include "test_server.h"

#include <gmock/gmock.h>


using namespace testing;
using namespace miral::toolkit;
using namespace std::chrono_literals;
using miral::WindowManagerTools;

namespace
{
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
auto const mir_window_get_buffer_stream = mir_surface_get_buffer_stream;
#endif

std::string const top_level{"top level"};
std::string const dialog{"dialog"};
std::string const tip{"tip"};

struct Workspaces : public miral::TestServer
{
    auto create_window(Connection const& connection, std::string const& name) -> Window
    {
        auto const spec = WindowSpec::for_normal_window(connection, 50, 50, mir_pixel_format_argb_8888)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_name(name.c_str());

        Window const window{spec.create_window()};
        client_windows[name] = window;
        mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(window));

        return window;
    }

    auto create_tip(Connection const& connection, std::string const& name, Window const& parent) -> Window
    {
        MirRectangle aux_rect{10, 10, 10, 10};
        auto const spec = WindowSpec::for_tip(connection, 50, 50, mir_pixel_format_argb_8888, parent, &aux_rect, mir_edge_attachment_any)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_name(name.c_str());

        Window const window{spec.create_window()};
        client_windows[name] = window;
        mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(window));

        return window;
    }

    auto create_dialog(Connection const& connection, std::string const& name, Window const& parent) -> Window
    {
        auto const spec = WindowSpec::for_dialog(connection, 50, 50, mir_pixel_format_argb_8888, parent)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_name(name.c_str());

        Window const window{spec.create_window()};
        client_windows[name] = window;
        mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(window));

        return window;
    }

    auto create_workspace() -> std::shared_ptr<miral::Workspace>
    {
        invoke_tools([this](WindowManagerTools& tools)
            { workspaces.push_back(tools.create_workspace()); });

        return workspaces.back();
    }

    void SetUp() override
    {
        miral::TestServer::SetUp();
        client_connection  = connect_client("Workspaces");
        create_window(client_connection, top_level);
        create_dialog(client_connection, dialog, client_windows[top_level]);
        create_tip(client_connection, tip, client_windows[dialog]);

        EXPECT_THAT(client_windows.size(), Eq(3u));
        EXPECT_THAT(server_windows.size(), Eq(3u));
    }

    void TearDown() override
    {
        client_windows.clear();
        client_connection.reset();
        workspaces.clear();
        miral::TestServer::TearDown();
    }

    Connection client_connection;

    auto server_window(std::string const& key) -> miral::Window
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        return server_windows[key];
    }

    auto client_window(std::string const& key) -> Window
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        return client_windows[key];
    }

    auto windows_in_workspace(std::shared_ptr<miral::Workspace> const& workspace) -> std::vector<miral::Window>
    {
        std::vector<miral::Window> result;

        auto enumerate = [&result](miral::Window const& window)
            {
                result.push_back(window);
            };

        invoke_tools([&](WindowManagerTools& tools)
            { tools.for_each_window_in_workspace(workspace, enumerate); });

        return result;
    }

private:
    std::mutex mutable mutex;
    std::map<std::string, Window> client_windows;
    std::vector<std::shared_ptr<miral::Workspace>> workspaces;
    std::map<std::string, miral::Window> server_windows;

    struct WorkspacesWindowManagerPolicy : miral::TestServer::TestWindowManagerPolicy
    {
        WorkspacesWindowManagerPolicy(WindowManagerTools const& tools, Workspaces& test_fixture) :
            TestWindowManagerPolicy(tools, test_fixture), test_fixture{test_fixture} {}

        void advise_new_window(miral::WindowInfo const& window_info) override
        {
            miral::TestServer::TestWindowManagerPolicy::advise_new_window(window_info);

            std::lock_guard<decltype(test_fixture.mutex)> lock{test_fixture.mutex};
            test_fixture.server_windows[window_info.name()] = window_info.window();
        }

        Workspaces& test_fixture;
    };

    auto build_window_manager_policy(WindowManagerTools const& tools)
    -> std::unique_ptr<TestWindowManagerPolicy> override
    {
        return std::make_unique<WorkspacesWindowManagerPolicy>(tools, *this);
    }
};
}

TEST_F(Workspaces, before_a_tree_is_added_to_workspace_it_is_empty)
{
    auto const workspace = create_workspace();

    EXPECT_THAT(windows_in_workspace(workspace).size(), Eq(0u));
}

TEST_F(Workspaces, when_a_tree_is_added_to_workspace_all_surfaces_in_tree_are_added)
{
    auto const workspace = create_workspace();
    invoke_tools([&, this](WindowManagerTools& tools)
                     { tools.add_tree_to_workspace(server_window(dialog), workspace); });

    EXPECT_THAT(windows_in_workspace(workspace).size(), Eq(3u));
}

TEST_F(Workspaces, when_a_tree_is_removed_from_workspace_all_surfaces_in_tree_are_removed)
{
    auto const workspace = create_workspace();
    invoke_tools([&, this](WindowManagerTools& tools)
                     { tools.add_tree_to_workspace(server_window(dialog), workspace); });

    invoke_tools([&, this](WindowManagerTools& tools)
                     { tools.remove_tree_from_workspace(server_window(tip), workspace); });

    EXPECT_THAT(windows_in_workspace(workspace).size(), Eq(0u));
}