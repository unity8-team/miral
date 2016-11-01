/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mirapplication.h"

#include <mir/scene/session.h>

std::string qtmir::name_of(const miral::Application &application)
{
    return application->name();
}

pid_t qtmir::pid_of(const miral::Application &application)
{
    return application->process_id();
}

void qtmir::apply_lifecycle_state_to(const miral::Application &application, MirLifecycleState state)
{
    application->set_lifecycle_state(state);
}
