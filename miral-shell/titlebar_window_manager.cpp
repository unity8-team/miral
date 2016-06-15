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
#include "titlebar/titlebar_user_data.h"

#include <miral/application_info.h>
#include <miral/internal_client.h>
#include <miral/window_info.h>
#include <miral/window_manager_tools.h>

#include <linux/input.h>

#include <mutex>
#include <condition_variable>

using namespace miral;

namespace
{
int const title_bar_height = 10;
Size titlebar_size_for_window(Size window_size)
{
    return {window_size.width, Height{title_bar_height}};
}

Point titlebar_position_for_window(Point window_position)
{
    return {
        window_position.x,
        window_position.y - DeltaY(title_bar_height)
    };
}
}

struct TitlebarWindowManagerPolicy::TitlebarProvider
{
    ~TitlebarProvider()
    {
        notify_done();
    }

    void operator()(MirConnection* connection)
    {
        std::unique_lock<decltype(mutex)> lock{mutex};
        this->connection = connection;
        cv.wait(lock, [this] { return done; });
    }

    void operator()(std::weak_ptr<mir::scene::Session> const& session)
    {
        std::unique_lock<decltype(mutex)> lock{mutex};
        this->weak_session = session;
    }

    auto session() const -> std::shared_ptr<mir::scene::Session>
    {
        std::unique_lock<decltype(mutex)> lock{mutex};
        return weak_session.lock();
    }

    void notify_done()
    {
        std::unique_lock<decltype(mutex)> lock{mutex};
        done = true;
        cv.notify_one();
    }

private:
    std::mutex mutable mutex;
    MirConnection* connection = nullptr;
    std::weak_ptr<mir::scene::Session> weak_session;

    std::condition_variable cv;
    bool done = false;
};

TitlebarWindowManagerPolicy::TitlebarWindowManagerPolicy(
    WindowManagerTools* const tools,
    SpinnerSplash const& spinner,
    miral::InternalClientLauncher const& launcher) :
    CanonicalWindowManagerPolicy(tools),
    tools(tools),
    spinner{spinner},
    titlebar_provider{std::make_unique<TitlebarProvider>()}
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
            // TODO this is a rather roundabout way to detect a titlebar
            if (auto const possible_titlebar = tools->window_at(old_cursor))
            {
                if (auto const parent = tools->info_for(possible_titlebar).parent())
                {
                    if (auto const& parent_userdata =
                        std::static_pointer_cast<TitlebarUserData>(tools->info_for(parent).userdata()))
                    {
                        if (possible_titlebar == parent_userdata->window)
                        {
                            if (auto const target = tools->window_at(old_cursor))
                            {
                                tools->select_active_window(target);
                                tools->drag_active_window(cursor - old_cursor);
                            }
                            consumes_event = true;
                        }
                    }
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

    if (!window_info.needs_titlebar(window_info.type()))
        return;

    Window const& window = window_info.window();

    auto format = mir_pixel_format_xrgb_8888;
    WindowSpecification params;
    params.size() = titlebar_size_for_window(window.size());
    params.name() = "decoration";
    params.pixel_format() = format;
    params.buffer_usage() = WindowSpecification::BufferUsage::software;
    params.top_left() = titlebar_position_for_window(window.top_left());
    params.type() = mir_surface_type_gloss;

    auto& titlebar_info = tools->build_window(window.application(), params);
    titlebar_info.window().set_alpha(0.9);
    titlebar_info.parent(window);

    auto data = std::make_shared<TitlebarUserData>(titlebar_info.window());
    window_info.userdata(data);
    window_info.add_child(titlebar_info.window());
}

void TitlebarWindowManagerPolicy::advise_focus_lost(WindowInfo const& info)
{
    CanonicalWindowManagerPolicy::advise_focus_lost(info);

    if (auto const titlebar = std::static_pointer_cast<TitlebarUserData>(info.userdata()))
    {
        titlebar->paint_titlebar(0x3F);
    }
}

void TitlebarWindowManagerPolicy::advise_focus_gained(WindowInfo const& info)
{
    CanonicalWindowManagerPolicy::advise_focus_gained(info);

    if (auto const titlebar = std::static_pointer_cast<TitlebarUserData>(info.userdata()))
    {
        titlebar->paint_titlebar(0xFF);
    }

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

    if (auto const titlebar = std::static_pointer_cast<TitlebarUserData>(window_info.userdata()))
    {
        switch (state)
        {
        case mir_surface_state_restored:
            titlebar->window.resize(titlebar_size_for_window(window_info.restore_rect().size));
            titlebar->window.show();
            break;

        case mir_surface_state_maximized:
        case mir_surface_state_vertmaximized:
        case mir_surface_state_hidden:
        case mir_surface_state_minimized:
            titlebar->window.hide();
            break;

        case mir_surface_state_horizmaximized:
            titlebar->window.resize(titlebar_size_for_window({display_area.size.width, window_info.restore_rect().size.height}));
            titlebar->window.show();
            break;

        case mir_surface_state_fullscreen:
        default:
            break;
        }
    }
}

void TitlebarWindowManagerPolicy::advise_resize(WindowInfo const& window_info, Size const& new_size)
{
    CanonicalWindowManagerPolicy::advise_resize(window_info, new_size);

    if (auto const titlebar = std::static_pointer_cast<TitlebarUserData>(window_info.userdata()))
    {
        titlebar->window.resize({new_size.width, Height{title_bar_height}});
    }
}

void TitlebarWindowManagerPolicy::advise_delete_window(WindowInfo const& window_info)
{
    CanonicalWindowManagerPolicy::advise_delete_window(window_info);

    if (auto const titlebar = std::static_pointer_cast<TitlebarUserData>(window_info.userdata()))
    {
        tools->destroy(titlebar->window);
    }
}

void TitlebarWindowManagerPolicy::handle_displays_updated(Rectangles const& displays)
{
    CanonicalWindowManagerPolicy::handle_displays_updated(displays);

    display_area = displays.bounding_rectangle();
}
