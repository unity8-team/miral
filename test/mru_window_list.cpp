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

#include <miral/window.h>

#include <mir/test/doubles/stub_surface.h>
#include <mir/test/doubles/stub_session.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace miral
{
class MRUWindowList
{
public:

    void note(Window const& window);
    void erase(Window const& window);
    auto top() const -> Window;

private:
    std::vector<Window> surfaces;
};
}

/////////////////////

void miral::MRUWindowList::note(Window const& window)
{
    surfaces.push_back(window);
}

auto miral::MRUWindowList::top() const -> Window
{
    return (!surfaces.empty()) ? surfaces.front() : Window{};
}

/////////////////////

using StubSurface = mir::test::doubles::StubSurface;
using namespace testing;

namespace
{
struct StubSession : mir::test::doubles::StubSession
{
    StubSession(int number_of_surfaces)
    {
        for (auto i = 0; i != number_of_surfaces; ++i)
            surfaces.push_back(std::make_shared<mir::test::doubles::StubSurface>());
    }

    std::shared_ptr<mir::scene::Surface> surface(mir::frontend::SurfaceId surface) const override
    {
        return surfaces.at(surface.as_value());
    }

    std::vector<std::shared_ptr<StubSurface>> surfaces;
};

MATCHER(IsNullWindow, std::string("is not null"))
{
    return !arg;
}
}

struct MRUWindowList : testing::Test
{
    miral::MRUWindowList mru_list;

    std::shared_ptr<StubSession> const stub_session{std::make_shared<StubSession>(3)};
    miral::Application app{stub_session};
    miral::Window window_a{app, mir::frontend::SurfaceId{0}};
    miral::Window window_b{app, mir::frontend::SurfaceId{1}};
    miral::Window window_c{app, mir::frontend::SurfaceId{2}};
};

TEST_F(MRUWindowList, when_created_is_empty)
{
    EXPECT_THAT(mru_list.top(), IsNullWindow());
}

TEST_F(MRUWindowList, given_one_window_pushed_that_window_is_top)
{
    mru_list.note(window_a);
    EXPECT_THAT(mru_list.top(), Eq(window_a));
}

