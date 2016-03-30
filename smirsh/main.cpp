/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
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

#include "window_management.h"

#include "miral/display_configuration_option.h"
#include "miral/runner.h"

namespace me = mir::examples;

int main(int argc, char const* argv[])
{
    miral::MirRunner runner(argc, argv, "smirsh.config");

    return runner.run_with(
        {
            me::window_manager_option,
            miral::display_configuration_options
        });
}
