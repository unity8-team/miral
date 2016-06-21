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

#include "mru_window_list.h"

#include <algorithm>

void miral::MRUWindowList::push(Window const& window)
{
    surfaces.erase(remove(begin(surfaces), end(surfaces), window), end(surfaces));
    surfaces.push_back(window);
}

void miral::MRUWindowList::erase(Window const& window)
{
    surfaces.erase(remove(begin(surfaces), end(surfaces), window), end(surfaces));
}

auto miral::MRUWindowList::top() const -> Window
{
    return (!surfaces.empty()) ? surfaces.back() : Window{};
}

void miral::MRUWindowList::enumerate(Enumerator const& enumerator) const
{
    for (auto i = surfaces.rbegin(); i != surfaces.rend(); ++i)
        if (!enumerator(const_cast<Window&>(*i)))
            break;
}
