/*
 * Copyright © 2014-2016 Canonical Ltd.
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
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#include "window_management.h"

#include "tiling_window_manager.h"
#include "canonical_window_manager.h"

#include "mir/al/basic_window_manager.h"

#include "mir/abnormal_exit.h"
#include "mir/server.h"
#include "mir/options/option.h"
#include "mir/shell/system_compositor_window_manager.h"

namespace me = mir::examples;
namespace msh = mir::shell;

// Demonstrate introducing a window management strategy
namespace
{
char const* const wm_option = "window-manager";
char const* const wm_system_compositor = "system-compositor";

struct WindowManagerOption
{
    std::string const name;
    auto (*build)(miral::WindowManagerTools* tools) -> std::unique_ptr<miral::WindowManagementPolicy>;
};

WindowManagerOption options[] =
    {
        { "canonical", [](miral::WindowManagerTools* tools) -> std::unique_ptr<miral::WindowManagementPolicy>
            { return std::make_unique<me::CanonicalWindowManagerPolicy>(tools); }
        },
        { "tiling",    [](miral::WindowManagerTools* tools) -> std::unique_ptr<miral::WindowManagementPolicy>
            { return std::make_unique<me::TilingWindowManagerPolicy>(tools); }
        }
    };
}

void me::window_manager_option(Server& server)
{
    std::string description = "window management strategy [{";

    for (auto const& option : ::options)
        description += option.name + "|";

    description += "system-compositor}]";

    server.add_configuration_option(wm_option, description, ::options[0].name);

    server.override_the_window_manager_builder([&server](msh::FocusController* focus_controller)
        -> std::shared_ptr<msh::WindowManager>
        {
            auto const options = server.get_options();
            auto const selection = options->get<std::string>(wm_option);

            for (auto const& option : ::options)
                if (selection == option.name)
                {
                    return std::make_shared<mir::al::BasicWindowManager>
                        (focus_controller, server.the_shell_display_layout(), option.build);
                }

            if (selection == wm_system_compositor)
            {
                return std::make_shared<msh::SystemCompositorWindowManager>(
                    focus_controller,
                    server.the_shell_display_layout(),
                    server.the_session_coordinator());
            }

            throw mir::AbnormalExit("Unknown window manager: " + selection);
        });
}
