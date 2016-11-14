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

#include "miral/debugextensions.h"

#include "coordinate_translator.h"

#include <mir/server.h>
#include <mir/options/option.h>

class miral::DebugExtensions::Self : public CoordinateTranslator
{
};

miral::DebugExtensions::DebugExtensions() :
    self{std::make_shared<Self>()}
{
}

miral::DebugExtensions::DebugExtensions(DebugExtensions const&) = default;
auto miral::DebugExtensions::operator=(DebugExtensions const&) -> DebugExtensions& = default;

void miral::DebugExtensions::enable() { self->enable_debug_api(); }
void miral::DebugExtensions::disable() { self->disable_debug_api(); }

void miral::DebugExtensions::operator()(mir::Server& server) const
{
    server.override_the_coordinate_translator([&server, this]
          {
            // Using a text constant as mir::options::debug_opt isn't published by libmirserver-dev
            if (server.get_options()->is_set("debug"))
                self->enable_debug_api();

              return self;
          });
}
