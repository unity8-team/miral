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

#include "../miral/persistent_surface_store.h"

#include <miral/toolkit/persistent_id.h>

#include <mir/test/doubles/wrap_shell_to_track_latest_surface.h>
#include <mir_test_framework/connected_client_with_a_surface.h>
#include <mir/version.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace msh = mir::shell;
namespace ms = mir::scene;
namespace mt = mir::test;
namespace mtd = mt::doubles;
namespace mtf = mir_test_framework;

using namespace mir::geometry;
using namespace testing;

namespace
{
struct PersistentSurfaceStore : mtf::ConnectedClientWithASurface
{
    void SetUp() override
    {
        server.wrap_shell([this](std::shared_ptr<msh::Shell> const& wrapped)
            {
                auto const msc = std::make_shared<mtd::WrapShellToTrackLatestSurface>(wrapped);
                shell = msc;
                return msc;
            });

#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 25, 0)
        preset_display({}); // Workaround for lp:1611337
#endif
        persistent_surface_store(server);
        mtf::ConnectedClientWithASurface::SetUp();
    }

    void TearDown() override
    {
        mtf::ConnectedClientWithASurface::TearDown();
    }

    std::shared_ptr<ms::Surface> latest_shell_surface() const
    {
        auto myshell = shell.lock();
        EXPECT_THAT(myshell, NotNull());
        auto const result = myshell->latest_surface.lock();
        EXPECT_THAT(result, NotNull());
        return result;
    }

    miral::PersistentSurfaceStore persistent_surface_store;

private:
    std::weak_ptr<mtd::WrapShellToTrackLatestSurface> shell;
};
}

TEST_F(PersistentSurfaceStore, server_and_client_persistent_id_matches)
{
    auto const shell_server_surface = latest_shell_surface();
    ASSERT_THAT(shell_server_surface, NotNull());

    miral::toolkit::PersistentId client_surface_id{surface};

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 24, 0)
    auto const server_surface_id = persistent_surface_store.id_for_surface(shell_server_surface);

    auto const client_surface_id_string = client_surface_id.c_str();

    ASSERT_THAT(server_surface_id, Eq(client_surface_id_string));
#else
    EXPECT_THROW(persistent_surface_store.id_for_surface(shell_server_surface), std::logic_error);
#endif
}

TEST_F(PersistentSurfaceStore, server_can_identify_surface_specified_by_client)
{
    miral::toolkit::PersistentId client_surface_id{surface};

    auto const server_surface = persistent_surface_store.surface_for_id(client_surface_id.c_str());

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 24, 0)
    ASSERT_THAT(server_surface, Eq(latest_shell_surface()));
#else
    ASSERT_THAT(server_surface, IsNull());
#endif
}
