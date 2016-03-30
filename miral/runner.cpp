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

#include "mir/al/runner.h"

#include "mir/server.h"
#include "input_event_filter.h"
#include "mir/main_loop.h"
#include "mir/report_exception.h"

namespace ma = mir::al;


ma::MirRunner::MirRunner(int argc, char const* argv[], char const* config_file) :
    argc(argc), argv(argv), config_file{config_file}
{}

auto ma::MirRunner::run(std::initializer_list<void (*)(mir::Server&)> options)
-> int
try
{
    mir::Server server;

    server.set_config_filename(config_file);

    for (auto* option : options)
        option(server);

    // Create some input filters (we need to keep them or they deactivate)
    auto const quit_filter = make_quit_filter_for(server);

    // Provide the command line and run the server
    server.set_command_line(argc, argv);
    server.apply_settings();
    server.run();

    return server.exited_normally() ? EXIT_SUCCESS : EXIT_FAILURE;
}
catch (...)
{
    mir::report_exception();
    return EXIT_FAILURE;
}
