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

#include "active_outputs.h"
#include "output.h"

#include <mir/graphics/display_configuration_report.h>
#include <mir/graphics/display_configuration.h>
#include <mir/server.h>

#include <vector>

void miral::ActiveOutputsListener::advise_begin() {}
void miral::ActiveOutputsListener::advise_end() {}
void miral::ActiveOutputsListener::advise_create_output(Output const& /*output*/) {}
void miral::ActiveOutputsListener::advise_update_output(Output const& /*updated*/, Output const& /*original*/) {}
void miral::ActiveOutputsListener::advise_delete_output(Output const& /*output*/) {}

struct miral::ActiveOutputsMonitor::Self : mir::graphics::DisplayConfigurationReport
{
    virtual void initial_configuration(mir::graphics::DisplayConfiguration const& configuration) override;
    virtual void new_configuration(mir::graphics::DisplayConfiguration const& configuration) override;

    std::vector<ActiveOutputsListener*> listeners;
};

miral::ActiveOutputsMonitor::ActiveOutputsMonitor() :
    self{std::make_shared<Self>()}
{
}

miral::ActiveOutputsMonitor::~ActiveOutputsMonitor() = default;
miral::ActiveOutputsMonitor::ActiveOutputsMonitor(ActiveOutputsMonitor const&) = default;
miral::ActiveOutputsMonitor& miral::ActiveOutputsMonitor::operator=(ActiveOutputsMonitor const&) = default;

void miral::ActiveOutputsMonitor::add_listener(ActiveOutputsListener* listener)
{
    self->listeners.push_back(listener);
}

void miral::ActiveOutputsMonitor::delete_listener(ActiveOutputsListener* /*listener*/) { /*TODO*/ }

void miral::ActiveOutputsMonitor::operator()(mir::Server& server)
{
    server.override_the_display_configuration_report([this]{ return self; });
}

void miral::ActiveOutputsMonitor::Self::initial_configuration(mir::graphics::DisplayConfiguration const& configuration)
{
    new_configuration(configuration);
}

void miral::ActiveOutputsMonitor::Self::new_configuration(mir::graphics::DisplayConfiguration const& configuration)
{
    for (auto const l : listeners)
    {
        l->advise_begin();
        configuration.for_each_output([l](mir::graphics::DisplayConfigurationOutput const& output)
            { l->advise_create_output(Output(output)); });
        l->advise_end();
    }
}
