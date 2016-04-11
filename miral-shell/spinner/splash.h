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

#ifndef MIRAL_SHELL_SPINNER_SPLASH_H
#define MIRAL_SHELL_SPINNER_SPLASH_H

#include <mir_toolkit/client_types.h>

#include <memory>

namespace mir { namespace scene { class Session; }}

void spinner_splash(MirConnection* connection);
void spinner_server_notification(std::weak_ptr<mir::scene::Session> const session);
auto spinner_session() -> std::shared_ptr<mir::scene::Session>;

#endif //MIRAL_SHELL_SPINNER_SPLASH_H
