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

#include "test_server.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace
{
struct StopCallbacks : miral::TestServer
{
    void SetUp() override
    {
        runner.add_stop_callback([this] { callback(); });
        miral::TestServer::SetUp();
    }

    MOCK_METHOD0(callback, void());
};
}

TEST_F(StopCallbacks, are_called)
{
    EXPECT_CALL(*this, callback());
}