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

#include <functional>
#include <initializer_list>
#include <memory>

namespace mir { class Server; }

/** Mir Abstraction Layer.
 * A thin, hopefully ABI stable, layer over the Mir libraries exposing only
 * those abstractions needed to write a shell. One day this may inspire a core
 * Mir library.
 */
namespace miral
{

/// Runner for applying initialization options to Mir.
class MirRunner
{
public:
    MirRunner(int argc, char const* argv[]);
    MirRunner(int argc, char const* argv[], char const* config_file);
    ~MirRunner();

    auto run_with(std::initializer_list<std::function<void(::mir::Server&)>> options) -> int;

private:
    MirRunner(MirRunner const&) = delete;
    MirRunner& operator=(MirRunner const&) = delete;
    struct Self;
    std::unique_ptr<Self> self;
};
}

#endif //MIRAL_RUNNER_H
