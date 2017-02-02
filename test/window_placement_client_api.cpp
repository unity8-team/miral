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

#include <mir/version.h>
#include <mir_toolkit/version.h>

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 25, 0)
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
#include <mir_toolkit/events/surface_placement.h>
#else
#include <mir_toolkit/events/window_placement.h>
#endif
#endif

#include <miral/detail/mir_forward_compatibility.h>
#include <miral/toolkit/window_spec.h>
#include <miral/toolkit/window.h>

#include <mir/test/signal.h>
#include "test_server.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std::literals::chrono_literals;

using namespace testing;
namespace mt = mir::test;
namespace mtf = mir_test_framework;

using namespace miral::toolkit;

namespace
{
struct WindowPlacementClientAPI : miral::TestServer
{
    void SetUp() override
    {
        miral::TestServer::SetUp();

        char const* const test_name = __PRETTY_FUNCTION__;

        connection = connect_client(test_name);
        auto spec = WindowSpec::for_normal_window(connection, 400, 400, mir_pixel_format_argb_8888)
            .set_name(test_name);

        parent = spec.create_surface();
    }

    void TearDown() override
    {
        child.reset();
        parent.reset();
        connection.reset();

        miral::TestServer::TearDown();
    }

    Connection connection;
    Window parent;
    Window child;
};
}

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 25, 0)

#if MIR_CLIENT_VERSION == MIR_VERSION_NUMBER(3, 4, 0)
auto const mir_event_type_window_placement   = mir_event_type_surface_placement;
#endif

namespace
{
struct CheckPlacement
{
    CheckPlacement(int left, int top, unsigned int width, unsigned int height) :
        expected{left, top, width, height} {}

    void check(MirWindowPlacementEvent const* placement_event)
    {
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
        auto relative_position = mir_surface_placement_get_relative_position(placement_event);
#else
        auto relative_position = mir_window_placement_get_relative_position(placement_event);
#endif
        EXPECT_THAT(relative_position.top, Eq(expected.top));
        EXPECT_THAT(relative_position.left, Eq(expected.left));
        EXPECT_THAT(relative_position.height, Eq(expected.height));
        EXPECT_THAT(relative_position.width, Eq(expected.width));

        received.raise();
    }

    static void callback(MirWindow* /*surface*/, MirEvent const* event, void* context)
    {
        if (mir_event_get_type(event) == mir_event_type_window_placement)
        {
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
            auto const placement_event = mir_event_get_surface_placement_event(event);
#else
            auto const placement_event = mir_event_get_window_placement_event(event);
#endif
            static_cast<CheckPlacement*>(context)->check(placement_event);
        }
    }

    ~CheckPlacement()
    {
        EXPECT_TRUE(received.wait_for(400ms));
    }

private:
    MirRectangle const expected;
    mt::Signal received;
};
}

// It would be nice to verify creation and movement placement notifications in separate tests,
// However, to avoid a racy test, we need to detect both anyway. This seems like a good trade-off.
TEST_F(WindowPlacementClientAPI, given_menu_placements_away_from_edges_when_notified_result_is_as_requested)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    int const dx = 30;
    int const dy = 40;

    // initial placement
    {
        MirRectangle aux_rect{10, 20, 3, 4};
        CheckPlacement expected{aux_rect.left+(int)aux_rect.width, aux_rect.top, dx, dy};

        auto const spec = WindowSpec::
            for_menu(connection, dx, dy, mir_pixel_format_argb_8888, parent, &aux_rect, mir_edge_attachment_any)
            .set_event_handler(&CheckPlacement::callback, &expected)
            .set_name(test_name);

        child = spec.create_surface();
    }

    // subsequent movement
    {
        MirRectangle aux_rect{50, 60, 5, 7};
        CheckPlacement expected{aux_rect.left-dx, aux_rect.top, dx, dy};

        auto const spec = WindowSpec::for_changes(connection)
            .set_event_handler(&CheckPlacement::callback, &expected)
            .set_placement(&aux_rect, mir_placement_gravity_northwest, mir_placement_gravity_northeast, mir_placement_hints_flip_x, 0, 0);

        spec.apply_to(child);
    }
}
#endif
