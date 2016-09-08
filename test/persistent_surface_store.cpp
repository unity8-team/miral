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

#include <miral/application_info.h>

#include <mir/version.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_server.h"

using namespace testing;


struct PersistentSurfaceId : public miral::TestServer
{
    miral::Window get_first_window()
    {
        auto app = tools.find_application([&](miral::ApplicationInfo const& /*info*/){return true;});
        auto app_info = tools.info_for(app);
        return app_info.windows()[0];
    }
};

#include <mir/scene/surface.h>

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 24, 0)
TEST_F(PersistentSurfaceId, server_can_identify_window_specified_by_client)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    using namespace miral::toolkit;

    auto const connection = connect_client(test_name);
    auto const spec = SurfaceSpec::for_normal_surface(connection, 50, 50, mir_pixel_format_argb_8888)
        .set_name(test_name);

    Surface const surface{spec.create_surface()};

    miral::toolkit::PersistentId client_surface_id{surface};

    tools.invoke_under_lock([&]
        {
            auto const& window_info = tools.info_for_window_id(client_surface_id.c_str());

            ASSERT_TRUE(window_info.window());
            ASSERT_THAT(window_info.name(), Eq(test_name));
        });
}

TEST_F(PersistentSurfaceId, server_returns_correct_id_for_window)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    using namespace miral::toolkit;

    auto const connection = connect_client(test_name);
    auto const spec = SurfaceSpec::for_normal_surface(connection, 50, 50, mir_pixel_format_argb_8888)
        .set_name(test_name);

    Surface const surface{spec.create_surface()};

    miral::toolkit::PersistentId client_surface_id{surface};

    tools.invoke_under_lock([&]
        {
            auto window = get_first_window();
            auto id = tools.id_for_window(window);

            ASSERT_THAT(client_surface_id.c_str(), Eq(id));
        });
}
#else
TEST_F(PersistentSurfaceId, server_fails_gracefully_to_identify_window_specified_by_client)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    using namespace miral::toolkit;

    auto const connection = connect_client(test_name);
    auto const spec = SurfaceSpec::for_normal_surface(connection, 50, 50, mir_pixel_format_argb_8888)
        .set_name(test_name);

    Surface const surface{spec.create_surface()};

    miral::toolkit::PersistentId client_surface_id{surface};

    tools.invoke_under_lock([&]
        {
            EXPECT_THROW(tools.info_for_window_id(client_surface_id.c_str()), std::runtime_error);
        });
}

TEST_F(PersistentSurfaceId, server_fails_gracefully_to_return_id_for_window)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    using namespace miral::toolkit;

    auto const connection = connect_client(test_name);
    auto const spec = SurfaceSpec::for_normal_surface(connection, 50, 50, mir_pixel_format_argb_8888)
        .set_name(test_name);

    Surface const surface{spec.create_surface()};

    miral::toolkit::PersistentId client_surface_id{surface};

    tools.invoke_under_lock([&]
        {
            auto window = get_first_window();
            EXPECT_THROW(tools.id_for_window(window), std::runtime_error);
        });
}
#endif

TEST_F(PersistentSurfaceId, server_fails_gracefully_to_identify_window_from_garbage_id)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    using namespace miral::toolkit;

    auto const connection = connect_client(test_name);
    auto const spec = SurfaceSpec::for_normal_surface(connection, 50, 50, mir_pixel_format_argb_8888)
        .set_name(test_name);

    Surface const surface{spec.create_surface()};

    miral::toolkit::PersistentId client_surface_id{surface};

    tools.invoke_under_lock([&]
        {
            EXPECT_THROW(tools.info_for_window_id("garbage"), std::exception);
        });
}

TEST_F(PersistentSurfaceId, server_fails_gracefully_when_id_for_null_window_requested)
{
    tools.invoke_under_lock([&]
        {
            miral::Window window;
            EXPECT_THROW(tools.id_for_window(window), std::runtime_error);
        });
}
