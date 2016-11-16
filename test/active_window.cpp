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

#include <miral/toolkit/surface.h>
#include <miral/toolkit/surface_spec.h>
#include <mir_toolkit/mir_buffer_stream.h>

#include <miral/application_info.h>

#include <mir/test/signal.h>

#include <gtest/gtest.h>

using namespace testing;
using namespace miral::toolkit;

namespace
{
class FocusChangeSync
{
public:
    void exec(std::function<void()> const& f)
    {
        signal.reset();
        f();
        signal.wait_for(std::chrono::milliseconds(100));
    }

    static void raise_signal_on_focus_change(MirSurface* /*surface*/, MirEvent const* event, void* context)
    {
        if (mir_event_get_type(event) == mir_event_type_surface &&
            mir_surface_event_get_attribute(mir_event_get_surface_event(event)) == mir_surface_attrib_focus)
        {
            ((FocusChangeSync*)context)->signal.raise();
        }
    }
private:
    mir::test::Signal signal;
};

struct ActiveWindow : public miral::TestServer
{
    FocusChangeSync sync;
    
    auto create_surface(Connection const& connection, char const* name) -> Surface
    {
        auto const spec = SurfaceSpec::for_normal_surface(connection, 50, 50, mir_pixel_format_argb_8888)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_event_handler(&FocusChangeSync::raise_signal_on_focus_change, &sync)
            .set_name(name);

        Surface const surface{spec.create_surface()};

        sync.exec([&]{ mir_buffer_stream_swap_buffers_sync(mir_surface_get_buffer_stream(surface)); });

        return surface;
    }
};
}

TEST_F(ActiveWindow, a_single_window_when_ready_becomes_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const surface = create_surface(connection, test_name);

    invoke_tools([&](miral::WindowManagerTools& tools)
        {
            auto const window = tools.active_window();
            ASSERT_TRUE(window);
            auto const& window_info = tools.info_for(window);
            ASSERT_THAT(window_info.name(), Eq(test_name));
        });
}

TEST_F(ActiveWindow, a_single_window_when_hiding_becomes_inactive)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);
    auto const surface = create_surface(connection, test_name);

    sync.exec([&]{ mir_surface_set_state(surface, mir_surface_state_hidden); });

    invoke_tools([&](miral::WindowManagerTools& tools)
        {
            auto const window = tools.active_window();
            ASSERT_FALSE(window);
        });
}

TEST_F(ActiveWindow, a_single_window_when_unhiding_becomes_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);
    auto const surface = create_surface(connection, test_name);
    sync.exec([&]{ mir_surface_set_state(surface, mir_surface_state_hidden); });

    sync.exec([&]{ mir_surface_set_state(surface, mir_surface_state_restored); });

    invoke_tools([&](miral::WindowManagerTools& tools)
        {
            auto const window = tools.active_window();
            ASSERT_TRUE(window);
            auto const& window_info = tools.info_for(window);
            ASSERT_THAT(window_info.name(), Eq(test_name));
        });
}
