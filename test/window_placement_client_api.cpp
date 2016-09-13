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

#include <mir_toolkit/events/surface_placement.h>

#include <miral/toolkit/surface_spec.h>
#include <miral/toolkit/surface.h>

#include <mir/version.h>

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
        auto spec = SurfaceSpec::for_normal_surface(connection, 400, 400, mir_pixel_format_argb_8888)
            .set_name(test_name);

        parent = Surface{spec.create_surface()};
    }

    void TearDown() override
    {
        child.reset();
        parent.reset();
        connection.reset();

        miral::TestServer::TearDown();
    }

    Connection connection;
    Surface parent;
    Surface child;
};
}

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 25, 0)

namespace
{
struct PlacementCheck
{
    PlacementCheck(MirRectangle const& placement) : expected_rect{placement} {}

    void check(MirSurfacePlacementEvent const* placement_event)
    {
        EXPECT_THAT(mir_surface_placement_get_relative_position(placement_event).top, Eq(expected_rect.top));
        EXPECT_THAT(mir_surface_placement_get_relative_position(placement_event).left, Eq(expected_rect.left));
        EXPECT_THAT(mir_surface_placement_get_relative_position(placement_event).height, Eq(expected_rect.height));
        EXPECT_THAT(mir_surface_placement_get_relative_position(placement_event).width, Eq(expected_rect.width));

        received.raise();
    }

    ~PlacementCheck()
    {
        EXPECT_TRUE(received.wait_for(400ms));
    }

private:
    MirRectangle const expected_rect;
    mt::Signal received;
};

void surface_placement_event_callback(MirSurface* /*surface*/, MirEvent const* event, void* context)
{
    if (mir_event_get_type(event) == mir_event_type_surface_placement)
    {
        auto const placement_event = mir_event_get_surface_placement_event(event);
        static_cast<PlacementCheck*>(context)->check(placement_event);
    }
}
}

TEST_F(WindowPlacementClientAPI, given_placement_away_from_edges_when_notified_result_is_as_requested)
{
    char const* const test_name = __PRETTY_FUNCTION__;

    MirRectangle aux_rect{10, 20, 3, 4};

    int const dx = 30;
    int const dy = 40;

    PlacementCheck expected_placement{MirRectangle{aux_rect.left+(int)aux_rect.width, aux_rect.top, dx, dy}};

    auto spec = SurfaceSpec::for_for_menu(connection, dx, dy, mir_pixel_format_argb_8888, parent, &aux_rect, mir_edge_attachment_any)
        .set_event_handler(surface_placement_event_callback, &expected_placement)
        .set_name(test_name);

    spec.create_surface();
}
#endif

