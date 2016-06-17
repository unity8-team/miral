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

#include "titlebar_window_manager.h"
#include "titlebar_provider.h"

#include <miral/application_info.h>
#include <miral/internal_client.h>
#include <miral/window_info.h>
#include <miral/window_manager_tools.h>

#include <linux/input.h>

using namespace miral;


TitlebarWindowManagerPolicy::TitlebarWindowManagerPolicy(
    WindowManagerTools* const tools,
    SpinnerSplash const& spinner,
    miral::InternalClientLauncher const& launcher) :
    CanonicalWindowManagerPolicy(tools),
    tools(tools),
    spinner{spinner},
    titlebar_provider{std::make_unique<TitlebarProvider>(tools)}
{
    launcher.launch("decorations", *titlebar_provider);
}

TitlebarWindowManagerPolicy::~TitlebarWindowManagerPolicy() = default;

bool TitlebarWindowManagerPolicy::handle_pointer_event(MirPointerEvent const* event)
{
    auto consumes_event = CanonicalWindowManagerPolicy::handle_pointer_event(event);

    auto const action = mir_pointer_event_action(event);
    auto const modifiers = mir_pointer_event_modifiers(event) & modifier_mask;
    Point const cursor{
        mir_pointer_event_axis_value(event, mir_pointer_axis_x),
        mir_pointer_event_axis_value(event, mir_pointer_axis_y)};


    if (!consumes_event && action == mir_pointer_action_motion && !modifiers)
    {
        if (mir_pointer_event_button_state(event, mir_pointer_button_primary))
        {
            if (auto const possible_titlebar = tools->window_at(old_cursor))
            {
                if (possible_titlebar.application() == titlebar_provider->session())
                {
                    auto const& info = tools->info_for(possible_titlebar);
                    tools->select_active_window(info.parent());
                    tools->drag_active_window(cursor - old_cursor);
                    consumes_event = true;
                }
            }
        }
    }

    old_cursor = cursor;
    return consumes_event;
}


void TitlebarWindowManagerPolicy::advise_new_window(WindowInfo& window_info)
{
    CanonicalWindowManagerPolicy::advise_new_window(window_info);

    auto const application = window_info.window().application();

    if (application == titlebar_provider->session())
    {
        titlebar_provider->advise_new_titlebar(window_info);
        return;
    }

    if (application == spinner.session() || !window_info.needs_titlebar(window_info.type()))
        return;

    titlebar_provider->create_titlebar_for(window_info.window());
}

void TitlebarWindowManagerPolicy::advise_focus_lost(WindowInfo const& info)
{
    CanonicalWindowManagerPolicy::advise_focus_lost(info);

    titlebar_provider->paint_titlebar_for(info.window(), 0x3F);
}

void TitlebarWindowManagerPolicy::advise_focus_gained(WindowInfo const& info)
{
    CanonicalWindowManagerPolicy::advise_focus_gained(info);

    titlebar_provider->paint_titlebar_for(info.window(), 0xFF);

    // Frig to force the spinner to the top
    if (auto const spinner_session = spinner.session())
    {
        auto const& spinner_info = tools->info_for(spinner_session);

        if (spinner_info.windows().size() > 0)
            tools->raise_tree(spinner_info.windows()[0]);
    }
}

void TitlebarWindowManagerPolicy::advise_state_change(WindowInfo const& window_info, MirSurfaceState state)
{
    CanonicalWindowManagerPolicy::advise_state_change(window_info, state);

    titlebar_provider->advise_state_change(window_info, state, display_area);
}

void TitlebarWindowManagerPolicy::advise_resize(WindowInfo const& window_info, Size const& new_size)
{
    CanonicalWindowManagerPolicy::advise_resize(window_info, new_size);

    titlebar_provider->resize_titlebar_for(window_info.window(), new_size);
}

void TitlebarWindowManagerPolicy::advise_delete_window(WindowInfo const& window_info)
{
    CanonicalWindowManagerPolicy::advise_delete_window(window_info);

    titlebar_provider->destroy_titlebar_for(window_info.window());
}

void TitlebarWindowManagerPolicy::handle_displays_updated(Rectangles const& displays)
{
    CanonicalWindowManagerPolicy::handle_displays_updated(displays);

    display_area = displays.bounding_rectangle();
}

bool TitlebarWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
{
    if (miral::CanonicalWindowManagerPolicy::handle_keyboard_event(event))
        return true;

    // TODO this is a workaround for the lack of a way to detect server exit (Mir bug lp:1593655)
    // We need to exit the titlebar_provider "client" thread before the server exits
    auto const action = mir_keyboard_event_action(event);
    auto const scan_code = mir_keyboard_event_scan_code(event);
    auto const modifiers = mir_keyboard_event_modifiers(event) & modifier_mask;

    if (action == mir_keyboard_action_down && scan_code == KEY_BACKSPACE &&
        (modifiers == (mir_input_event_modifier_alt | mir_input_event_modifier_ctrl)))
    {
        titlebar_provider->stop();
    }

    return false;
}
