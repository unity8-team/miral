/*
 * Copyright © 2012-2016 Canonical Ltd.
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
#include "input_event_filter.h"

#include "mir/server.h"
#include "mir/main_loop.h"
#include "mir/graphics/default_display_configuration_policy.h"

#include "mir/report_exception.h"
#include "mir/options/option.h"

#include <chrono>

namespace me = mir::examples;
namespace mg = mir::graphics;

namespace
{
void add_timeout_option_to(mir::Server& server)
{
    static const char* const timeout_opt = "timeout";
    static const char* const timeout_descr = "Seconds to run before exiting";

    server.add_configuration_option(timeout_opt, timeout_descr, mir::OptionType::integer);

    server.add_init_callback([&server]
    {
        const auto options = server.get_options();
        if (options->is_set(timeout_opt))
        {
            static auto const exit_action = server.the_main_loop()->create_alarm([&server] { server.stop(); });
            exit_action->reschedule_in(std::chrono::seconds(options->get<int>(timeout_opt)));
        }
    });
}

void set_sidebyside_display(mir::Server& server)
{
    server.wrap_display_configuration_policy(
        [&](std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> const& /*wrapped*/)
            -> std::shared_ptr<mir::graphics::DisplayConfigurationPolicy>
            { return std::make_shared<mir::graphics::SideBySideDisplayConfigurationPolicy>(); });
}
}

int main(int argc, char const* argv[])
try
{
    mir::Server server;

    server.set_config_filename("smirsh.config");

    me::add_window_manager_option_to(server);
    add_timeout_option_to(server);
    set_sidebyside_display(server);

    // Create some input filters (we need to keep them or they deactivate)
    auto const quit_filter = me::make_quit_filter_for(server);

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
