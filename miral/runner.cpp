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

#include "miral/runner.h"

#include <mir/server.h>
#include <mir/report_exception.h>
#include <mir/options/option.h>

#include <iostream>
#include <cstring>

namespace
{
inline auto filename(std::string path) -> std::string
{
    return path.substr(path.rfind('/')+1);
}
}

miral::MirRunner::MirRunner(int argc, char const* argv[]) :
    argc(argc), argv(argv), config_file(filename(argv[0]) + ".config")
{
}

miral::MirRunner::MirRunner(int argc, char const* argv[], char const* config_file) :
    argc(argc), argv(argv), config_file{config_file}
{
}

namespace
{
// TODO There ought to be a better, generic way to specify startup applications
auto const gnome_terminal = "gnome-terminal-on-startup";

void enable_startup_applications(::mir::Server& server)
{
    server.add_configuration_option(gnome_terminal, "launch gnome-terminal on startup", mir::OptionType::null);
}

void launch_gnome_terminal()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        throw std::runtime_error("Failed to fork process");
    }

    if (pid == 0)
    {
        unsetenv("DISPLAY");                                // Discourage toolkits from using X11
        setenv("GDK_BACKEND", "mir", true);                 // configure GTK to use Mir
        setenv("QT_QPA_PLATFORM", "ubuntumirclient", true); // configure Qt to use Mir
        setenv("SDL_VIDEODRIVER", "mir", true);             // configure SDL to use Mir

        char const* exec_args[] = { "gnome-terminal", "--app-id", "com.canonical.miral.Terminal", nullptr };

        sleep(2);
        execvp(exec_args[0], const_cast<char*const*>(exec_args));

        throw std::logic_error(std::string("Failed to execute client (") + exec_args[0] + ") error: " + strerror(errno));
    }
}

void launch_startup_applications(::mir::Server& server)
{
    if (auto const options = server.get_options())
        if (options->is_set(gnome_terminal))
            launch_gnome_terminal();
}
}

auto miral::MirRunner::run_with(std::initializer_list<std::function<void(::mir::Server&)>> options)
-> int
try
{
    mir::Server server;

    server.set_config_filename(config_file);

    enable_startup_applications(server);

    for (auto& option : options)
        option(server);

    // Provide the command line and run the server
    server.set_command_line(argc, argv);
    server.apply_settings();

    launch_startup_applications(server);

    server.run();

    return server.exited_normally() ? EXIT_SUCCESS : EXIT_FAILURE;
}
catch (...)
{
    mir::report_exception();
    return EXIT_FAILURE;
}
