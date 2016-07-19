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

#include "../miral/active_outputs.h"
#include "../miral/output.h"

#include "mir_test_framework/headless_test.h"

#include "mir/test/doubles/fake_display.h"
#include "mir/test/fake_shared.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace mg = mir::graphics;
namespace mt = mir::test;
namespace mtd = mir::test::doubles;
namespace mtf = mir_test_framework;

using namespace miral;
using namespace testing;

namespace
{
struct MockActiveOutputsListener : ActiveOutputsListener
{
    MOCK_METHOD0(advise_begin, void());
    MOCK_METHOD0(advise_end, void());

    MOCK_METHOD1(advise_create_output, void(Output const&));
    MOCK_METHOD2(advise_update_output, void(Output const&, Output const&));
    MOCK_METHOD1(advise_delete_output, void(Output const&));
};

std::vector<Rectangle> const output_rects{{{0,0}, {640,480}}};

struct ActiveOutputs : mtf::HeadlessTest
{
    ActiveOutputs()
    {
        add_to_environment("MIR_SERVER_NO_FILE", "");
    }

    void SetUp() override
    {
        mtf::HeadlessTest::SetUp();
        preset_display(mt::fake_shared(display));
        active_outputs_monitor(server);
        active_outputs_monitor.add_listener(&active_outputs_listener);
    }

    void TearDown() override
    {
        active_outputs_monitor.delete_listener(&active_outputs_listener);
        mtf::HeadlessTest::TearDown();
    }

    mtd::FakeDisplay display{output_rects};
    ActiveOutputsMonitor active_outputs_monitor;
    NiceMock<MockActiveOutputsListener> active_outputs_listener;
};
}

TEST_F(ActiveOutputs, on_startup_listener_is_advised)
{
    EXPECT_CALL(active_outputs_listener, advise_create_output(_)).Times(AtLeast(1));
    start_server();
    stop_server();
}

TEST_F(ActiveOutputs, on_stopping_listener_is_advised)
{
    start_server();

    EXPECT_CALL(active_outputs_listener, advise_delete_output(_)).Times(AtLeast(1));
    stop_server();
}
