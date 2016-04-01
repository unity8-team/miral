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
char const* const wm_description = "window management strategy [{canonical|tiling|system-compositor}]";

char const* const wm_tiling = "tiling";
char const* const wm_canonical = "canonical";
char const* const wm_system_compositor = "system-compositor";
}

using CanonicalWindowManager = me::WindowManagerBuilder<me::CanonicalWindowManagerPolicy>;
using TilingWindowManager = me::WindowManagerBuilder<me::TilingWindowManagerPolicy>;

void me::window_manager_option(Server& server)
{
    server.add_configuration_option(wm_option, wm_description, wm_canonical);

    server.override_the_window_manager_builder([&server](msh::FocusController* focus_controller)
        -> std::shared_ptr<msh::WindowManager>
        {
            auto const options = server.get_options();
            auto const selection = options->get<std::string>(wm_option);

            if (selection == wm_tiling)
            {
                return std::make_shared<TilingWindowManager>(focus_controller);
            }
            else if (selection == wm_canonical)
            {
                return std::make_shared<CanonicalWindowManager>(focus_controller, server.the_shell_display_layout());
            }
            else if (selection == wm_system_compositor)
            {
                return std::make_shared<msh::SystemCompositorWindowManager>(
                    focus_controller,
                    server.the_shell_display_layout(),
                    server.the_session_coordinator());
            }

            throw mir::AbnormalExit("Unknown window manager: " + selection);
        });
}
