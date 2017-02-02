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

#include <miral/toolkit/window.h>
#include <miral/toolkit/window_spec.h>
#include <mir_toolkit/mir_buffer_stream.h>
#include <mir_toolkit/version.h>

#include <miral/application_info.h>

#include <mir/test/signal.h>

#include <gtest/gtest.h>

using namespace testing;
using namespace miral::toolkit;
using namespace std::chrono_literals;
using miral::WindowManagerTools;

namespace
{
class FocusChangeSync
{
public:
    void exec(std::function<void()> const& f)
    {
        signal.reset();
        f();
        signal.wait_for(100ms);
    }

    static void raise_signal_on_focus_change(MirWindow* /*surface*/, MirEvent const* event, void* context)
    {
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
        if (mir_event_get_type(event) == mir_event_type_surface &&
            mir_surface_event_get_attribute(mir_event_get_surface_event(event)) == mir_surface_attrib_focus)
#else
        if (mir_event_get_type(event) == mir_event_type_window &&
            mir_window_event_get_attribute(mir_event_get_window_event(event)) == mir_window_attrib_focus)
#endif
        {
            ((FocusChangeSync*)context)->signal.raise();
        }
    }

    auto signal_raised() -> bool { return signal.raised(); }

private:
    mir::test::Signal signal;
};

struct ActiveWindow : public miral::TestServer
{
    FocusChangeSync sync1;
    FocusChangeSync sync2;

    auto create_surface(Connection const& connection, char const* name, FocusChangeSync& sync) -> Window
    {
        auto const spec = WindowSpec::for_normal_window(connection, 50, 50, mir_pixel_format_argb_8888)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_event_handler(&FocusChangeSync::raise_signal_on_focus_change, &sync)
            .set_name(name);

        Window const surface{spec.create_window()};

#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        sync.exec([&]{ mir_buffer_stream_swap_buffers_sync(mir_surface_get_buffer_stream(surface)); });
#else
        sync.exec([&]{ mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(surface)); });
#endif
        EXPECT_TRUE(sync.signal_raised());

        return surface;
    }

#if MIR_CLIENT_VERSION >= MIR_VERSION_NUMBER(3, 4, 0)
    auto create_tip(Connection const& connection, char const* name, Window const& parent, FocusChangeSync& sync) -> Window
    {
        MirRectangle aux_rect{10, 10, 10, 10};
        auto const spec = WindowSpec::for_tip(connection, 50, 50, mir_pixel_format_argb_8888, parent, &aux_rect, mir_edge_attachment_any)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_event_handler(&FocusChangeSync::raise_signal_on_focus_change, &sync)
            .set_name(name);

        Window const surface{spec.create_window()};

        // Expect this to timeout: A tip should not receive focus
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        sync.exec([&]{ mir_buffer_stream_swap_buffers_sync(mir_surface_get_buffer_stream(surface)); });
#else
        sync.exec([&]{ mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(surface)); });
#endif
        EXPECT_FALSE(sync.signal_raised());

        return surface;
    }
#endif

    auto create_dialog(Connection const& connection, char const* name, Window const& parent, FocusChangeSync& sync) -> Window
    {
        auto const spec = WindowSpec::for_dialog(connection, 50, 50, mir_pixel_format_argb_8888, parent)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_event_handler(&FocusChangeSync::raise_signal_on_focus_change, &sync)
            .set_name(name);

        Window const surface{spec.create_window()};

#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
        sync.exec([&]{ mir_buffer_stream_swap_buffers_sync(mir_surface_get_buffer_stream(surface)); });
#else
        sync.exec([&]{ mir_buffer_stream_swap_buffers_sync(mir_window_get_buffer_stream(surface)); });
#endif
        EXPECT_TRUE(sync.signal_raised());

        return surface;
    }

    void assert_no_active_window()
    {
        invoke_tools([&](WindowManagerTools& tools)
            {
                auto const window = tools.active_window();
                ASSERT_FALSE(window);
            });
    }

    void assert_active_window_is(char const* const name)
    {
        invoke_tools([&](WindowManagerTools& tools)
            {
                 auto const window = tools.active_window();
                 ASSERT_TRUE(window);
                 auto const& window_info = tools.info_for(window);
                 ASSERT_THAT(window_info.name(), Eq(name));
            });
    }
};

auto const another_name = "second";
}

TEST_F(ActiveWindow, a_single_window_when_ready_becomes_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const surface = create_surface(connection, test_name, sync1);

    assert_active_window_is(test_name);
}

TEST_F(ActiveWindow, a_single_window_when_hiding_becomes_inactive)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);
    auto const surface = create_surface(connection, test_name, sync1);

#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
    sync1.exec([&]{ mir_surface_set_state(surface, mir_surface_state_hidden); });
#else
    sync1.exec([&]{ mir_window_set_state(surface, mir_window_state_hidden); });
#endif

    EXPECT_TRUE(sync1.signal_raised());
    assert_no_active_window();
}

TEST_F(ActiveWindow, a_single_window_when_unhiding_becomes_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);
    auto const surface = create_surface(connection, test_name, sync1);
#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
    sync1.exec([&]{ mir_surface_set_state(surface, mir_surface_state_hidden); });

    sync1.exec([&]{ mir_surface_set_state(surface, mir_surface_state_restored); });
#else
    sync1.exec([&]{ mir_window_set_state(surface, mir_window_state_hidden); });

    sync1.exec([&]{ mir_window_set_state(surface, mir_window_state_restored); });
#endif
    EXPECT_TRUE(sync1.signal_raised());

    assert_active_window_is(test_name);
}

TEST_F(ActiveWindow, a_second_window_when_ready_becomes_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const first_surface = create_surface(connection, "first", sync1);
    auto const surface = create_surface(connection, test_name, sync2);

    assert_active_window_is(test_name);
}

TEST_F(ActiveWindow, a_second_window_hiding_makes_first_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const first_surface = create_surface(connection, test_name, sync1);
    auto const surface = create_surface(connection, another_name, sync2);

#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
    sync2.exec([&]{ mir_surface_set_state(surface, mir_surface_state_hidden); });
#else
    sync2.exec([&]{ mir_window_set_state(surface, mir_window_state_hidden); });
#endif

    EXPECT_TRUE(sync2.signal_raised());
    assert_active_window_is(test_name);
}

TEST_F(ActiveWindow, a_second_window_unhiding_leaves_first_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const first_surface = create_surface(connection, test_name, sync1);
    auto const surface = create_surface(connection, another_name, sync2);

#if MIR_CLIENT_VERSION <= MIR_VERSION_NUMBER(3, 4, 0)
    sync1.exec([&]{ mir_surface_set_state(surface, mir_surface_state_hidden); });

    // Expect this to timeout
    sync2.exec([&]{ mir_surface_set_state(surface, mir_surface_state_restored); });
#else
    sync1.exec([&]{ mir_window_set_state(surface, mir_window_state_hidden); });

    // Expect this to timeout
    sync2.exec([&]{ mir_window_set_state(surface, mir_window_state_restored); });
#endif
    EXPECT_THAT(sync2.signal_raised(), Eq(false));
    assert_active_window_is(test_name);
}

TEST_F(ActiveWindow, switching_from_a_second_window_makes_first_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const first_surface = create_surface(connection, test_name, sync1);
    auto const surface = create_surface(connection, another_name, sync2);

    sync1.exec([&]{ invoke_tools([](WindowManagerTools& tools){ tools.focus_next_within_application(); }); });

    EXPECT_TRUE(sync1.signal_raised());
    assert_active_window_is(test_name);
}

TEST_F(ActiveWindow, switching_from_a_second_application_makes_first_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);
    auto const second_connection = connect_client(another_name);

    auto const first_surface = create_surface(connection, test_name, sync1);
    auto const surface = create_surface(second_connection, another_name, sync2);

    sync1.exec([&]{ invoke_tools([](WindowManagerTools& tools){ tools.focus_next_application(); }); });

    EXPECT_TRUE(sync1.signal_raised());
    assert_active_window_is(test_name);
}

TEST_F(ActiveWindow, closing_a_second_application_makes_first_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const first_surface = create_surface(connection, test_name, sync1);

    sync1.exec([&]
        {
            auto const second_connection = connect_client(another_name);
            auto const surface = create_surface(second_connection, another_name, sync2);
            assert_active_window_is(another_name);
        });

    EXPECT_TRUE(sync1.signal_raised());
    assert_active_window_is(test_name);
}

#if MIR_CLIENT_VERSION >= MIR_VERSION_NUMBER(3, 4, 0)
TEST_F(ActiveWindow, selecting_a_tip_makes_parent_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const parent = create_surface(connection, test_name, sync1);

    miral::Window parent_window;
    invoke_tools([&](WindowManagerTools& tools){ parent_window = tools.active_window(); });

    // Steal the focus
    auto second_connection = connect_client(another_name);
    auto second_surface = create_surface(second_connection, another_name, sync2);

    auto const tip = create_tip(connection, "tip", parent, sync2);

    sync1.exec([&]
        {
            invoke_tools([&](WindowManagerTools& tools)
                { tools.select_active_window(*tools.info_for(parent_window).children().begin()); });
        });
    EXPECT_TRUE(sync1.signal_raised());

    assert_active_window_is(test_name);
}
#endif

TEST_F(ActiveWindow, selecting_a_parent_makes_dialog_active)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const dialog_name = "dialog";
    auto const connection = connect_client(test_name);

    auto const parent = create_surface(connection, test_name, sync1);

    miral::Window parent_window;
    invoke_tools([&](WindowManagerTools& tools){ parent_window = tools.active_window(); });

    auto const dialog = create_dialog(connection, dialog_name, parent, sync2);

    // Steal the focus
    auto second_connection = connect_client(another_name);
    auto second_surface = create_surface(second_connection, another_name, sync1);

    sync2.exec([&]{ invoke_tools([&](WindowManagerTools& tools){ tools.select_active_window(parent_window); }); });

    EXPECT_TRUE(sync2.signal_raised());
    assert_active_window_is(dialog_name);
}

TEST_F(ActiveWindow, input_methods_are_not_focussed)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const parent = create_surface(connection, test_name, sync1);
    auto const input_method = WindowSpec::for_input_method(connection, 50, 50, parent).create_window();

    assert_active_window_is(test_name);

    invoke_tools([&](WindowManagerTools& tools)
        {
            auto const& info = tools.info_for(tools.active_window());
            tools.select_active_window(info.children().at(0));
        });

    assert_active_window_is(test_name);
}

TEST_F(ActiveWindow, satellites_are_not_focussed)
{
    char const* const test_name = __PRETTY_FUNCTION__;
    auto const connection = connect_client(test_name);

    auto const parent = create_surface(connection, test_name, sync1);
    auto const satellite = WindowSpec::for_satellite(connection, 50, 50, parent).create_window();

    assert_active_window_is(test_name);

    invoke_tools([&](WindowManagerTools& tools)
    {
        auto const& info = tools.info_for(tools.active_window());
        tools.select_active_window(info.children().at(0));
    });

    assert_active_window_is(test_name);
}