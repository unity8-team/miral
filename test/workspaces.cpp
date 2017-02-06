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

struct Workspaces;

struct WorkspacesWindowManagerPolicy : miral::TestServer::TestWindowManagerPolicy, miral::WorkspacePolicy
{
    WorkspacesWindowManagerPolicy(WindowManagerTools const& tools, Workspaces& test_fixture);
    ~WorkspacesWindowManagerPolicy();

    void advise_new_window(miral::WindowInfo const& window_info) override;

    MOCK_METHOD2(advise_adding_to_workspace,
                 void(std::shared_ptr<miral::Workspace> const&, std::vector<miral::Window> const&));

    MOCK_METHOD2(advise_removing_from_workspace,
                 void(std::shared_ptr<miral::Workspace> const&, std::vector<miral::Window> const&));

    Workspaces& test_fixture;
};

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
        std::shared_ptr<miral::Workspace> result;

        invoke_tools([&](WindowManagerTools& tools)
            { result = tools.create_workspace(); });

        return result;
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

    auto workspaces_containing_window(miral::Window const& window) -> std::vector<std::shared_ptr<miral::Workspace>>
    {
        std::vector<std::shared_ptr<miral::Workspace>> result;

        auto enumerate = [&result](std::shared_ptr<miral::Workspace> const& workspace)
            {
                result.push_back(workspace);
            };

        invoke_tools([&](WindowManagerTools& tools)
            { tools.for_each_workspace_containing(window, enumerate); });

        return result;
    }

    auto policy() -> WorkspacesWindowManagerPolicy&
    {
        if (!the_policy) throw std::logic_error("the_policy isn't valid");
        return *the_policy;
    }

private:
    std::mutex mutable mutex;
    std::map<std::string, Window> client_windows;
    std::map<std::string, miral::Window> server_windows;
    WorkspacesWindowManagerPolicy* the_policy{nullptr};

    friend struct WorkspacesWindowManagerPolicy;

    auto build_window_manager_policy(WindowManagerTools const& tools)
    -> std::unique_ptr<TestWindowManagerPolicy> override
    {
        return std::make_unique<WorkspacesWindowManagerPolicy>(tools, *this);
    }
};

WorkspacesWindowManagerPolicy::WorkspacesWindowManagerPolicy(WindowManagerTools const& tools, Workspaces& test_fixture) :
TestWindowManagerPolicy(tools, test_fixture), test_fixture{test_fixture}
{
    test_fixture.the_policy = this;
}

WorkspacesWindowManagerPolicy::~WorkspacesWindowManagerPolicy()
{
    test_fixture.the_policy = nullptr;
}


void WorkspacesWindowManagerPolicy::advise_new_window(miral::WindowInfo const& window_info)
{
    miral::TestServer::TestWindowManagerPolicy::advise_new_window(window_info);

    std::lock_guard<decltype(test_fixture.mutex)> lock{test_fixture.mutex};
    test_fixture.server_windows[window_info.name()] = window_info.window();
}
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

TEST_F(Workspaces, given_a_tree_in_a_workspace_when_another_tree_is_added_and_removed_from_workspace_the_original_tree_remains)
{
    auto const workspace = create_workspace();
    auto const original_tree = "original_tree";
    auto const client_window = create_window(client_connection, original_tree);
    auto const original_window= server_window(original_tree);

    invoke_tools([&, this](WindowManagerTools& tools)
        { tools.add_tree_to_workspace(original_window, workspace); });

    invoke_tools([&, this](WindowManagerTools& tools)
        { tools.add_tree_to_workspace(server_window(top_level), workspace); });
    invoke_tools([&, this](WindowManagerTools& tools)
        { tools.remove_tree_from_workspace(server_window(top_level), workspace); });

    EXPECT_THAT(windows_in_workspace(workspace), ElementsAre(original_window));
}

TEST_F(Workspaces, when_a_tree_is_added_to_a_workspace_all_surfaces_are_contained_in_the_workspace)
{
    auto const workspace = create_workspace();
    invoke_tools([&, this](WindowManagerTools& tools)
        { tools.add_tree_to_workspace(server_window(dialog), workspace); });

    EXPECT_THAT(workspaces_containing_window(server_window(top_level)), ElementsAre(workspace));
    EXPECT_THAT(workspaces_containing_window(server_window(dialog)), ElementsAre(workspace));
    EXPECT_THAT(workspaces_containing_window(server_window(tip)), ElementsAre(workspace));
}


TEST_F(Workspaces, when_a_tree_is_added_to_a_workspaces_twice_surfaces_are_contained_in_one_workspace)
{
    auto const workspace = create_workspace();
    invoke_tools([&, this](WindowManagerTools& tools)
        {
            tools.add_tree_to_workspace(server_window(dialog), workspace);
            tools.add_tree_to_workspace(server_window(dialog), workspace);
        });

    EXPECT_THAT(workspaces_containing_window(server_window(top_level)), ElementsAre(workspace));
    EXPECT_THAT(workspaces_containing_window(server_window(dialog)), ElementsAre(workspace));
    EXPECT_THAT(workspaces_containing_window(server_window(tip)), ElementsAre(workspace));

    EXPECT_THAT(workspaces_containing_window(server_window(top_level)).size(), Eq(1u));
    EXPECT_THAT(workspaces_containing_window(server_window(dialog)).size(), Eq(1u));
    EXPECT_THAT(workspaces_containing_window(server_window(tip)).size(), Eq(1u));
}

TEST_F(Workspaces, when_a_tree_is_added_to_two_workspaces_all_surfaces_are_contained_in_two_workspaces)
{
    auto const workspace1 = create_workspace();
    auto const workspace2 = create_workspace();
    invoke_tools([&, this](WindowManagerTools& tools)
        {
            tools.add_tree_to_workspace(server_window(dialog), workspace1);
            tools.add_tree_to_workspace(server_window(dialog), workspace2);
        });

    EXPECT_THAT(workspaces_containing_window(server_window(top_level)).size(), Eq(2u));
    EXPECT_THAT(workspaces_containing_window(server_window(dialog)).size(), Eq(2u));
    EXPECT_THAT(workspaces_containing_window(server_window(tip)).size(), Eq(2u));
}

TEST_F(Workspaces, when_workspace_is_closed_surfaces_are_no_longer_contained_in_it)
{
    auto const workspace1 = create_workspace();
    auto workspace2 = create_workspace();
    invoke_tools([&, this](WindowManagerTools& tools)
        {
            tools.add_tree_to_workspace(server_window(dialog), workspace1);
            tools.add_tree_to_workspace(server_window(dialog), workspace2);
        });

    workspace2.reset();

    EXPECT_THAT(workspaces_containing_window(server_window(top_level)), ElementsAre(workspace1));
    EXPECT_THAT(workspaces_containing_window(server_window(dialog)), ElementsAre(workspace1));
    EXPECT_THAT(workspaces_containing_window(server_window(tip)), ElementsAre(workspace1));

    EXPECT_THAT(workspaces_containing_window(server_window(top_level)).size(), Eq(1u));
    EXPECT_THAT(workspaces_containing_window(server_window(dialog)).size(), Eq(1u));
    EXPECT_THAT(workspaces_containing_window(server_window(tip)).size(), Eq(1u));
}

TEST_F(Workspaces, when_a_tree_is_added_to_a_workspace_the_policy_is_notified)
{
    auto const workspace = create_workspace();

    EXPECT_CALL(policy(), advise_adding_to_workspace(workspace,
         ElementsAre(server_window(top_level), server_window(dialog), server_window(tip))));

    invoke_tools([&, this](WindowManagerTools& tools)
                     { tools.add_tree_to_workspace(server_window(dialog), workspace); });
}
