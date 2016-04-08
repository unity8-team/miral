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

#include "tiling_window_manager.h"
#include "canonical_window_manager.h"

#include "miral/display_configuration_option.h"
#include "miral/runner.h"
#include "miral/window_management_options.h"
#include "miral/quit_on_ctrl_alt_bksp.h"

#include <unistd.h>

#include <iostream>
#include <cstring>

int main(int argc, char const* argv[])
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

        execvp(exec_args[0], const_cast<char*const*>(exec_args));

        std::cerr << "Failed to execute client (" << exec_args[0] << ") error: " << strerror(errno) << '\n';
        return EXIT_FAILURE;
    }

    using namespace miral;

    return MirRunner{argc, argv}.run_with(
        {
            WindowManagerOptions
            {
                add_window_manager_policy<CanonicalWindowManagerPolicy>("canonical"),
                add_window_manager_policy<TilingWindowManagerPolicy>("tiling"),
            },
            display_configuration_options,
            QuitOnCtrlAltBkSp{},
        });
}
