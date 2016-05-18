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
};
}

/////////////////////

auto miral::MRUWindowList::top() const -> Window
{
    return {};
}

/////////////////////

namespace
{
MATCHER(IsNull, std::string("is not null"))
{
    return !arg;
}
}

struct MRUWindowList : testing::Test
{
    miral::MRUWindowList mru_list;

};

TEST_F(MRUWindowList, when_created_is_empty)
{
    EXPECT_THAT(mru_list.top(), IsNull());
}

