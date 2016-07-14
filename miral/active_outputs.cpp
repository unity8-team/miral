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


void miral::ActiveOutputsListener::advise_begin() {}
void miral::ActiveOutputsListener::advise_end() {}
void miral::ActiveOutputsListener::advise_create_output(Output const& /*output*/) {}
void miral::ActiveOutputsListener::advise_update_output(Output const& /*updated*/, Output const& /*original*/) {}
void miral::ActiveOutputsListener::advise_delete_output(Output const& /*output*/) {}

class miral::ActiveOutputsMonitor::Self { /*TODO*/ };

miral::ActiveOutputsMonitor::ActiveOutputsMonitor() :
    self{std::make_shared<Self>()}
{
}

miral::ActiveOutputsMonitor::~ActiveOutputsMonitor() = default;
miral::ActiveOutputsMonitor::ActiveOutputsMonitor(ActiveOutputsMonitor const&) = default;
miral::ActiveOutputsMonitor& miral::ActiveOutputsMonitor::operator=(ActiveOutputsMonitor const&) = default;

void miral::ActiveOutputsMonitor::add_listener(ActiveOutputsListener* /*listener*/) { /*TODO*/ }
void miral::ActiveOutputsMonitor::delete_listener(ActiveOutputsListener* /*listener*/) { /*TODO*/ }
void miral::ActiveOutputsMonitor::operator()(mir::Server& /*server*/) { /*TODO*/ }

