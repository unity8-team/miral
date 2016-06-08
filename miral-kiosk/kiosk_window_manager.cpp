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
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#include "kiosk_window_manager.h"

#include "miral/application_info.h"
#include "miral/window_info.h"
#include "miral/window_manager_tools.h"

#include <linux/input.h>

#include <algorithm>

namespace ms = mir::scene;
using namespace miral;

KioskWindowManagerPolicy::KioskWindowManagerPolicy(WindowManagerTools* const tools, SwSplash const& splash) :
    tools{tools},
    splash{splash}
{
}

void KioskWindowManagerPolicy::handle_app_info_updated(Rectangles const& /*displays*/)
{
}

void KioskWindowManagerPolicy::handle_displays_updated(Rectangles const& /*displays*/)
{
}

auto KioskWindowManagerPolicy::place_new_surface(
    miral::ApplicationInfo const& /*app_info*/,
    miral::WindowSpecification const& request_parameters)
-> miral::WindowSpecification
{
    auto parameters = request_parameters;

    Rectangle const active_display = tools->active_display();

    if (!parameters.parent().is_set() || !parameters.parent().value().lock())
    {
        parameters.top_left() = active_display.top_left;
        parameters.size() = active_display.size;
    }

    return parameters;
}

void KioskWindowManagerPolicy::advise_new_window(WindowInfo& /*window_info*/)
{
}

void KioskWindowManagerPolicy::handle_window_ready(WindowInfo& window_info)
{
    tools->select_active_window(window_info.window());
}

namespace
{
template<typename ValueType>
void reset(mir::optional_value<ValueType>& option)
{
    if (option.is_set()) option.consume();
}
}

void KioskWindowManagerPolicy::handle_modify_window(
    miral::WindowInfo& window_info,
    miral::WindowSpecification const& modifications)
{
    auto mods = modifications;

    // filter out changes we don't want the client making
    reset(mods.top_left());
    reset(mods.size());
    reset(mods.output_id());
    reset(mods.state());
    reset(mods.preferred_orientation());
    reset(mods.edge_attachment());
    reset(mods.min_width());
    reset(mods.min_height());
    reset(mods.max_width());
    reset(mods.max_height());
    reset(mods.width_inc());
    reset(mods.height_inc());
    reset(mods.min_aspect());
    reset(mods.max_aspect());
    reset(mods.parent());

    tools->modify_window(window_info, mods);
}

void KioskWindowManagerPolicy::advise_delete_window(WindowInfo const& /*window_info*/)
{
}

void KioskWindowManagerPolicy::handle_raise_window(WindowInfo& window_info)
{
    tools->select_active_window(window_info.window());
}

bool KioskWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
{
    auto const action = mir_keyboard_event_action(event);
    auto const scan_code = mir_keyboard_event_scan_code(event);
    auto const modifiers = mir_keyboard_event_modifiers(event) & modifier_mask;

    if (action == mir_keyboard_action_down &&
            modifiers == mir_input_event_modifier_alt &&
            scan_code == KEY_TAB)
    {
        tools->focus_next_application();

        return true;
    }
    else if (action == mir_keyboard_action_down &&
            modifiers == mir_input_event_modifier_alt &&
            scan_code == KEY_GRAVE)
    {
        tools->focus_next_within_application();

        return true;
    }

    return false;
}

bool KioskWindowManagerPolicy::handle_touch_event(MirTouchEvent const* event)
{
    auto const count = mir_touch_event_point_count(event);

    long total_x = 0;
    long total_y = 0;

    for (auto i = 0U; i != count; ++i)
    {
        total_x += mir_touch_event_axis_value(event, i, mir_touch_axis_x);
        total_y += mir_touch_event_axis_value(event, i, mir_touch_axis_y);
    }

    Point const cursor{total_x/count, total_y/count};

    tools->select_active_window(tools->window_at(cursor));

    return false;
}

bool KioskWindowManagerPolicy::handle_pointer_event(MirPointerEvent const* event)
{
    auto const action = mir_pointer_event_action(event);

    Point const cursor{
        mir_pointer_event_axis_value(event, mir_pointer_axis_x),
        mir_pointer_event_axis_value(event, mir_pointer_axis_y)};

    if (action == mir_pointer_action_button_down)
    {
        tools->select_active_window(tools->window_at(cursor));
    }

    return false;
}

void KioskWindowManagerPolicy::advise_focus_gained(WindowInfo const& info)
{
    tools->raise_tree(info.window());

    if (auto session = splash.session().lock())
    {
        auto const& app_info = tools->info_for(session);

        for (auto const& s : app_info.windows())
            tools->raise_tree(s);
    }
}

void KioskWindowManagerPolicy::advise_focus_lost(WindowInfo const& /*info*/)
{
}

void KioskWindowManagerPolicy::advise_state_change(WindowInfo const& /*window_info*/, MirSurfaceState /*state*/)
{
}

void KioskWindowManagerPolicy::advise_resize(WindowInfo const& /*window_info*/, Size const& /*new_size*/)
{
}

void KioskWindowManagerPolicy::advise_new_app(miral::ApplicationInfo& /*application*/)
{
}

void KioskWindowManagerPolicy::advise_delete_app(miral::ApplicationInfo const& /*application*/)
{
}

void KioskWindowManagerPolicy::advise_raise(std::vector<miral::Window> const& /*windows*/)
{
}
