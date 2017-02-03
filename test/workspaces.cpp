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

using namespace testing;
using namespace miral::toolkit;
using namespace std::chrono_literals;
using miral::WindowManagerTools;

namespace
{
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
auto const mir_window_get_buffer_stream = mir_surface_get_buffer_stream;
#endif

struct Workspaces : public miral::TestServer
{
    auto create_window(Connection const& connection, char const* name) -> Window
    {
        auto const spec = WindowSpec::for_normal_window(connection, 50, 50, mir_pixel_format_argb_8888)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_name(name);

        Window const surface{spec.create_window()};

        mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(surface));

        return surface;
    }

    auto create_tip(Connection const& connection, char const* name, Window const& parent) -> Window
    {
        MirRectangle aux_rect{10, 10, 10, 10};
        auto const spec = WindowSpec::for_tip(connection, 50, 50, mir_pixel_format_argb_8888, parent, &aux_rect, mir_edge_attachment_any)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_name(name);

        Window const surface{spec.create_window()};

        mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(surface));

        return surface;
    }

    auto create_dialog(Connection const& connection, char const* name, Window const& parent) -> Window
    {
        auto const spec = WindowSpec::for_dialog(connection, 50, 50, mir_pixel_format_argb_8888, parent)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_name(name);

        Window const surface{spec.create_window()};

        mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(surface));

        return surface;
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
        connection  = connect_client("Workspaces");
        top_level   = create_window(connection, "top level");
        dialog      = create_dialog(connection, "dialog", top_level);
        tip         = create_tip(connection, "tip", dialog);
    }

    void TearDown() override
    {
        tip.reset();
        dialog.reset();
        top_level.reset();
        connection.reset();
        miral::TestServer::TearDown();
    }

    Connection connection;
    Window top_level;
    Window dialog;
    Window tip;

    std::vector<std::shared_ptr<miral::Workspace>> workspaces;
};
}

TEST_F(Workspaces, when_a_tree_is_added_to_workspace_all_surfaces_in_tree_are_added)
{
    auto const workspace = create_workspace();


}