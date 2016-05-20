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

#include <miral/mru_window_list.h>

#include <mir/test/doubles/stub_surface.h>
#include <mir/test/doubles/stub_session.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

TEST_F(MRUWindowList, given_empty_list_when_a_window_pushed_that_window_is_top)
{
    mru_list.push(window_a);
    EXPECT_THAT(mru_list.top(), Eq(window_a));
}

TEST_F(MRUWindowList, given_non_empty_list_when_a_window_pushed_that_window_is_top)
{
    mru_list.push(window_a);
    mru_list.push(window_b);
    mru_list.push(window_c);
    EXPECT_THAT(mru_list.top(), Eq(window_c));
}

TEST_F(MRUWindowList, given_non_empty_list_when_top_window_is_erased_that_window_is_no_longer_on_top)
{
    mru_list.push(window_a);
    mru_list.push(window_b);
    mru_list.push(window_c);
    mru_list.erase(window_c);
    EXPECT_THAT(mru_list.top(), Ne(window_c));
}

TEST_F(MRUWindowList, a_window_pushed_twice_is_not_enumerated_twice)
{
    mru_list.push(window_a);
    mru_list.push(window_b);
    mru_list.push(window_a);

    int count{0};

    mru_list.enumerate([&](miral::Window& window)
        { if (window == window_a) ++count; return true; });

    EXPECT_THAT(count, Eq(1));
}

TEST_F(MRUWindowList, after_multiple_pushes_windows_are_enumerated_in_mru_order)
{
    mru_list.push(window_a);
    mru_list.push(window_b);
    mru_list.push(window_c);
    mru_list.push(window_a);
    mru_list.push(window_b);
    mru_list.push(window_a);

    mru_list.push(window_c);
    mru_list.push(window_b);
    mru_list.push(window_a);

    std::vector<miral::Window> as_enumerated;

    mru_list.enumerate([&](miral::Window& window)
       { as_enumerated.push_back(window); return true; });

    EXPECT_THAT(as_enumerated, ElementsAre(window_a, window_b, window_c));
}

TEST_F(MRUWindowList, when_enumerator_returns_false_enumeration_is_short_circuited)
{
    mru_list.push(window_a);
    mru_list.push(window_b);
    mru_list.push(window_c);

    int count{0};

    mru_list.enumerate([&](miral::Window&) { ++count; return false; });

    EXPECT_THAT(count, Eq(1));
}
