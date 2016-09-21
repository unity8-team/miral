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

#include "miral/set_window_managment_policy.h"
#include "basic_window_manager.h"
#include "window_management_trace.h"

#include <mir/server.h>
#include <mir/version.h>

namespace msh = mir::shell;

miral::SetWindowManagmentPolicy::SetWindowManagmentPolicy(WindowManagementPolicyBuilder const& builder) :
    builder{builder}
{
}

miral::SetWindowManagmentPolicy::~SetWindowManagmentPolicy() = default;

void miral::SetWindowManagmentPolicy::operator()(mir::Server& server) const
{
    server.override_the_window_manager_builder([this, &server](msh::FocusController* focus_controller)
        -> std::shared_ptr<msh::WindowManager>
        {
            auto const display_layout = server.the_shell_display_layout();

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 24, 0)
            auto const persistent_surface_store = server.the_persistent_surface_store();
#else
            std::shared_ptr<mir::shell::PersistentSurfaceStore> const persistent_surface_store;
#endif

//            return std::make_shared<BasicWindowManager>(focus_controller, display_layout, persistent_surface_store, builder);

            auto trace_builder = [this](WindowManagerTools const& tools) -> std::unique_ptr<miral::WindowManagementPolicy>
            {
                return std::make_unique<WindowManagementTrace>(tools, builder);
            };

            return std::make_shared<BasicWindowManager>(focus_controller, display_layout, persistent_surface_store, trace_builder);
        });
}
