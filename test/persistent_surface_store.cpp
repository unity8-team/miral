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

#include <miral/toolkit/persistent_id.h>
#include <miral/toolkit/surface.h>
#include <miral/toolkit/surface_spec.h>

#include <mir/version.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_server.h"

using namespace testing;


using PersistentSurfaceId = miral::TestServer;

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 24, 0)
TEST_F(PersistentSurfaceId, server_can_identify_window_specified_by_client)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    using namespace miral::toolkit;

    auto const connection = connect_client(test_name);
    auto const spec = SurfaceSpec::for_normal_surface(connection, 50, 50, mir_pixel_format_argb_8888);
    Surface const surface{spec.create_surface()};

    miral::toolkit::PersistentId client_surface_id{surface};

    tools.invoke_under_lock([&]
        {
            auto const& window_info = tools.info_for_window_id(client_surface_id.c_str());

            ASSERT_TRUE(window_info.window());
            ASSERT_THAT(window_info.name(), Eq(test_name));
        });
}
#endif