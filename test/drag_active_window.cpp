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

using namespace miral;
using namespace testing;
namespace mt = mir::test;

namespace
{
X const display_left{0};
Y const display_top{0};
Width  const display_width{640};
Height const display_height{480};

Rectangle const display_area{{display_left, display_top}, {display_width, display_height}};

auto const null_window = Window{};

auto create_surface(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params)
-> mir::frontend::SurfaceId
{
    // This type is Mir-internal, I hope we don't need to create it here
    std::shared_ptr<mir::frontend::EventSink> const sink;

    return session->create_surface(params, sink);
}

struct DragActiveWindow : TestWindowManagerTools
{
    Size const initial_parent_size{600, 400};

    Window window;

    WindowSpecification modification;

    void SetUp() override
    {
        basic_window_manager.add_display(display_area);

        mir::scene::SurfaceCreationParameters creation_parameters;
        basic_window_manager.add_session(session);

        EXPECT_CALL(*window_manager_policy, advise_new_window(_))
            .WillOnce(Invoke([this](WindowInfo const& window_info) { window = window_info.window(); }));

        creation_parameters.size = initial_parent_size;
        basic_window_manager.add_surface(session, creation_parameters, &create_surface);
        basic_window_manager.select_active_window(window);

        // Clear the expectations used to capture parent & child
        Mock::VerifyAndClearExpectations(window_manager_policy);
    }
};
}

TEST_F(DragActiveWindow, moves_normal_surface)
{
    Displacement const movement{10, 10};
    auto const initial_position = window.top_left();
    auto const expected_position = initial_position + movement;
    
    EXPECT_CALL(*window_manager_policy, advise_move_to(_, expected_position));

    window_manager_tools.drag_active_window(movement);
    EXPECT_THAT(window.top_left(), Eq(expected_position));
}