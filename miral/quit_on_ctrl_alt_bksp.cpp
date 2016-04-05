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

#include "miral/quit_on_ctrl_alt_bksp.h"

#include <mir/input/event_filter.h>
#include <mir/input/composite_event_filter.h>
#include <mir/server.h>

#include <linux/input.h>

#include <functional>

class miral::QuitOnCtrlAltBkSp::QuitFilter : public mir::input::EventFilter
{
public:
    QuitFilter(std::function<void()> const& quit_action);

    bool handle(MirEvent const& event) override;

private:
    std::function<void()> const quit_action;
};


miral::QuitOnCtrlAltBkSp::QuitFilter::QuitFilter(std::function<void()> const& quit_action)
    : quit_action{quit_action}
{
}

bool miral::QuitOnCtrlAltBkSp::QuitFilter::handle(MirEvent const& event)
{
    if (mir_event_get_type(&event) != mir_event_type_input)
        return false;

    MirInputEvent const* input_event = mir_event_get_input_event(&event);
    if (mir_input_event_get_type(input_event) != mir_input_event_type_key)
        return false;

    MirKeyboardEvent const* kev = mir_input_event_get_keyboard_event(input_event);
    if (mir_keyboard_event_action(kev) != mir_keyboard_action_down)
        return false;

    MirInputEventModifiers mods = mir_keyboard_event_modifiers(kev);
    if (!(mods & mir_input_event_modifier_alt) || !(mods & mir_input_event_modifier_ctrl))
        return false;

    if (mir_keyboard_event_scan_code(kev) != KEY_BACKSPACE)
        return false;

    quit_action();
    return true;
}

void miral::QuitOnCtrlAltBkSp::operator()(mir::Server& server)
{
    quit_filter = std::make_shared<QuitFilter>([&]{ server.stop(); });

    server.add_init_callback([this, &server] { server.the_composite_event_filter()->append(quit_filter); });
}
