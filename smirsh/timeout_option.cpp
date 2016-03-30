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

#include "timeout_option.h"

#include "mir/main_loop.h"
#include "mir/options/option.h"
#include "mir/server.h"

#include <chrono>

namespace me = mir::examples;

void me::add_timeout_option_to(mir::Server& server)
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
