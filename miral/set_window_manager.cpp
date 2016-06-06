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

#include "miral/set_window_manager.h"
#include "basic_window_manager.h"

#include <mir/server.h>

namespace msh = mir::shell;

miral::SetWindowManager::SetWindowManager(WindowManagementPolicyBuilder const& builder) :
    builder{builder}
{
}

miral::SetWindowManager::~SetWindowManager() = default;

void miral::SetWindowManager::operator()(mir::Server& server) const
{
    server.override_the_window_manager_builder([this, &server](msh::FocusController* focus_controller)
        -> std::shared_ptr<msh::WindowManager>
        {
            auto const display_layout = server.the_shell_display_layout();

            return std::make_shared<BasicWindowManager>(focus_controller, display_layout, builder);
        });
}
