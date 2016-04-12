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

#include <chrono>
#include <thread>

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
auto const startup_apps = "startup-apps";

void enable_startup_applications(::mir::Server& server)
{
    server.add_configuration_option(startup_apps, "Colon separated list of startup apps", mir::OptionType::string);
}

void launch_startup_app(std::string socket_file, std::string app)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        throw std::runtime_error("Failed to fork process");
    }

    if (pid == 0)
    {
        auto const time_limit = std::chrono::steady_clock::now() + std::chrono::seconds(2);

        do
        {
            if (access(socket_file.c_str(), R_OK|W_OK) != -1)
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        while (std::chrono::steady_clock::now() < time_limit);

        unsetenv("DISPLAY");                                // Discourage toolkits from using X11
        setenv("GDK_BACKEND", "mir", true);                 // configure GTK to use Mir
        setenv("QT_QPA_PLATFORM", "ubuntumirclient", true); // configure Qt to use Mir
        setenv("SDL_VIDEODRIVER", "mir", true);             // configure SDL to use Mir

        // gnome-terminal is the (only known) special case
        char const* exec_args[] = { "gnome-terminal", "--app-id", "com.canonical.miral.Terminal", nullptr };

        if (app != exec_args[0])
        {
            exec_args[0] = app.c_str();
            exec_args[1] = nullptr;
        }

        execvp(exec_args[0], const_cast<char*const*>(exec_args));

        throw std::logic_error(std::string("Failed to execute client (") + exec_args[0] + ") error: " + strerror(errno));
    }
}

void launch_startup_applications(::mir::Server& server)
{
    if (auto const options = server.get_options())
    {
        if (options->is_set(startup_apps))
        {
            auto const socket_file = options->get<std::string>("file");
            auto const value = options->get<std::string>(startup_apps);

            for (auto i = begin(value); i != end(value); )
            {
                auto const j = find(i, end(value), ':');

                launch_startup_app(socket_file, std::string{i, j});

                if ((i = j) != end(value)) ++i;
            }
        }
    }
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

    // Has to be done after apply_settings() parses the command-line and
    // before run() starts allocates resources and starts threads.
    launch_startup_applications(server);

    server.run();

    return server.exited_normally() ? EXIT_SUCCESS : EXIT_FAILURE;
}
catch (...)
{
    mir::report_exception();
    return EXIT_FAILURE;
}
