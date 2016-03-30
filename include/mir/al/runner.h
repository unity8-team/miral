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

#ifndef MIRAL_RUNNER_H
#define MIRAL_RUNNER_H

#include <initializer_list>

namespace mir
{
class Server;

namespace al
{
class MirRunner
{
public:
    MirRunner(int argc, char const* argv[], char const* config_file);

    auto run(std::initializer_list<void (*)(::mir::Server&)> options) -> int;

private:
    int const argc;
    char const** argv;
    char const* const config_file;
};
}
}

#endif //MIRAL_RUNNER_H
