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
        client_window[name] = window;
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
        client_window[name] = window;
        mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(window));

        return window;
    }

    auto create_dialog(Connection const& connection, std::string const& name, Window const& parent) -> Window
    {
        auto const spec = WindowSpec::for_dialog(connection, 50, 50, mir_pixel_format_argb_8888, parent)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_name(name.c_str());

        Window const window{spec.create_window()};
        client_window[name] = window;
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
        create_dialog(client_connection, dialog, client_window[top_level]);
        create_tip(client_connection, tip, client_window[dialog]);

        EXPECT_THAT(client_window.size(), Eq(3u));
        EXPECT_THAT(server_window.size(), Eq(3u));
    }

    void TearDown() override
    {
        client_window.clear();
        client_connection.reset();
        workspaces.clear();
        miral::TestServer::TearDown();
    }

    Connection client_connection;
    std::map<std::string, Window> client_window;

    std::vector<std::shared_ptr<miral::Workspace>> workspaces;
    std::map<std::string, miral::Window> server_window;

    auto build_window_manager_policy(WindowManagerTools const& tools)
    -> std::unique_ptr<TestWindowManagerPolicy> override
    {
        struct WorkspacesWindowManagerPolicy : miral::TestServer::TestWindowManagerPolicy
        {
            WorkspacesWindowManagerPolicy(WindowManagerTools const& tools, Workspaces& test_fixture) :
                TestWindowManagerPolicy(tools, test_fixture), test_fixture{test_fixture} {}

            void advise_new_window(miral::WindowInfo const& window_info) override
            {
                miral::TestServer::TestWindowManagerPolicy::advise_new_window(window_info);

                test_fixture.server_window[window_info.name()] = window_info.window();
            }

            Workspaces& test_fixture;
        };

        return std::make_unique<WorkspacesWindowManagerPolicy>(tools, *this);
    }
};
}

TEST_F(Workspaces, before_a_tree_is_added_to_workspace_it_is_empty)
{
    auto const workspace = create_workspace();

    std::vector<miral::Window> windows_in_workspace;

    auto enumerate = [&windows_in_workspace](miral::Window const& window)
        {
            windows_in_workspace.push_back(window);
        };

    invoke_tools([&](WindowManagerTools& tools)
        { tools.for_each_window_in_workspace(workspace, enumerate); });

    EXPECT_THAT(windows_in_workspace.size(), Eq(0u));
}

TEST_F(Workspaces, when_a_tree_is_added_to_workspace_all_surfaces_in_tree_are_added)
{
    auto const workspace = create_workspace();

    std::vector<miral::Window> windows_in_workspace;

    auto enumerate = [&windows_in_workspace](miral::Window const& window)
        {
            windows_in_workspace.push_back(window);
        };

    invoke_tools([&, this](WindowManagerTools& tools)
        { tools.add_tree_to_workspace(server_window[dialog], workspace); });

    invoke_tools([&](WindowManagerTools& tools)
                     { tools.for_each_window_in_workspace(workspace, enumerate); });

    EXPECT_THAT(windows_in_workspace.size(), Eq(3u));
}