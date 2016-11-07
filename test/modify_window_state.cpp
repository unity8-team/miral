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

#include "test_window_manager_tools.h"
#include <mir/event_printer.h>

using namespace miral;
using namespace testing;
namespace mt = mir::test;
using mir::operator<<;

namespace
{
X const display_left{0};
Y const display_top{0};
Width const display_width{640};
Height const display_height{480};

Rectangle const display_area{{display_left,  display_top},
                             {display_width, display_height}};

auto const null_window = Window{};

struct ModifyWindowState : TestWindowManagerTools, WithParamInterface<MirSurfaceType>
{
    Size const initial_parent_size{600, 400};

    Window window;

    void SetUp() override
    {
        basic_window_manager.add_display(display_area);
        basic_window_manager.add_session(session);
    }

    void create_window_of_type(MirSurfaceType type)
    {
        mir::scene::SurfaceCreationParameters creation_parameters;
        creation_parameters.type = type;
        creation_parameters.size = initial_parent_size;

        EXPECT_CALL(*window_manager_policy, advise_new_window(_))
            .WillOnce(
                Invoke(
                    [this](WindowInfo const& window_info)
                        { window = window_info.window(); }));

        basic_window_manager.add_surface(session, creation_parameters, &create_surface);
        basic_window_manager.select_active_window(window);

        // Clear the expectations used to capture parent & child
        Mock::VerifyAndClearExpectations(window_manager_policy);
    }
};

using ForNormalSurface = ModifyWindowState;

TEST_P(ForNormalSurface, state)
{
    auto const original_state = mir_surface_state_restored;
    auto const new_state = MirSurfaceState(GetParam());
    auto const state_is_visible = (new_state != mir_surface_state_minimized) && (new_state != mir_surface_state_hidden);

    create_window_of_type(mir_surface_type_normal);
    auto const& info = window_manager_tools.info_for(window);

    WindowSpecification mods;
    mods.state() = new_state;
    window_manager_tools.modify_window(window, mods);
    EXPECT_THAT(std::shared_ptr<mir::scene::Surface>(window)->state(), Eq(new_state));
    EXPECT_THAT(info.state(), Eq(new_state));
    EXPECT_THAT(info.is_visible(), Eq(state_is_visible)) << "State is " << new_state;

    mods.state() = original_state;
    window_manager_tools.modify_window(window, mods);
    EXPECT_THAT(std::shared_ptr<mir::scene::Surface>(window)->state(), Eq(original_state));
    EXPECT_THAT(info.state(), Eq(original_state));
    EXPECT_TRUE(info.is_visible());
}
}

INSTANTIATE_TEST_CASE_P(ModifyWindowState, ForNormalSurface, ::testing::Values(
//    mir_surface_state_unknown,
    mir_surface_state_restored,
    mir_surface_state_minimized,
    mir_surface_state_maximized,
    mir_surface_state_vertmaximized,
    mir_surface_state_fullscreen,
    mir_surface_state_horizmaximized,
    mir_surface_state_hidden
//    mir_surface_states
));
