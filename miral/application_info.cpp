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

#include "miral/application_info.h"

struct miral::ApplicationInfo::Self
{
    std::vector<Window> windows;
    std::shared_ptr<void> userdata;
};

miral::ApplicationInfo::ApplicationInfo() :
    self{std::make_unique<Self>()}
{
}

miral::ApplicationInfo::~ApplicationInfo() = default;

miral::ApplicationInfo::ApplicationInfo(ApplicationInfo const& that) :
    self{std::make_unique<Self>(*that.self)}
{
}

auto miral::ApplicationInfo::operator=(ApplicationInfo const& that) -> ApplicationInfo&
{
    *self = *that.self;
    return *this;
}

auto miral::ApplicationInfo::windows() const -> std::vector <Window>&
{
    return self->windows;
}

void miral::ApplicationInfo::add_window(Window const& window)
{
    self->windows.push_back(window);
}

void miral::ApplicationInfo::remove_window(Window const& window)
{
    auto& siblings = self->windows;

    for (auto i = begin(siblings); i != end(siblings); ++i)
    {
        if (window == *i)
        {
            siblings.erase(i);
            break;
        }
    }
}
