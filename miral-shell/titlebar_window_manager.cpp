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

#include <miral/application_info.h>
#include <miral/internal_client.h>
#include <miral/window_info.h>
#include <miral/window_manager_tools.h>

#include <miral/toolkit/surface_spec.h>

#include <condition_variable>
#include <map>
#include <mutex>
#include <mir_toolkit/mir_buffer_stream.h>
#include <cstring>
#include <sstream>
#include <iostream>

using namespace miral;

namespace
{
int const title_bar_height = 10;
}

using namespace miral::toolkit;

struct TitlebarWindowManagerPolicy::TitlebarProvider
{
    TitlebarProvider(miral::WindowManagerTools* const tools) : tools{tools} {}
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
        std::lock_guard<decltype(mutex)> lock{mutex};
        this->weak_session = session;
    }

    auto session() const -> std::shared_ptr<mir::scene::Session>
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        return weak_session.lock();
    }

    void create_titlebar_for(Window const& window)
    {
        std::ostringstream buffer;

        buffer << std::shared_ptr<mir::scene::Surface>(window).get();

        auto const spec = SurfaceSpec::for_normal_surface(
            connection, window.size().width.as_int(), title_bar_height, mir_pixel_format_xrgb_8888)
            .set_buffer_usage(mir_buffer_usage_software)
            .set_type(mir_surface_type_gloss)
            .set_name(buffer.str().c_str());
            // Can we set alpha to 0.9?

        std::lock_guard<decltype(mutex)> lock{mutex};
        spec.create_surface(insert, &window_to_titlebar[window]);
    }

    void paint_titlebar_for(Window const& window, int intensity)
    {
        if (auto surface = find_titlebar_surface(window))
        {
            MirBufferStream* buffer_stream = mir_surface_get_buffer_stream(surface);
            MirGraphicsRegion region;
            mir_buffer_stream_get_graphics_region(buffer_stream, &region);

            char* row = region.vaddr;

            for (int j = 0; j < region.height; j++)
            {
                memset(row, intensity, 4*region.width);

                row += region.stride;
            }

            mir_buffer_stream_swap_buffers(buffer_stream, [](MirBufferStream*, void*) {}, nullptr);
        }
    }

    void destroy_titlebar_for(Window const& window)
    {
        std::lock_guard<decltype(mutex)> lock{mutex};

        auto const found = window_to_titlebar.find(window);

        if (found != window_to_titlebar.end())
        {
            if (auto surface = found->second.titlebar.load())
                mir_surface_release(surface, [](MirSurface*, void*) {}, nullptr);

            window_to_titlebar.erase(found);
        }
        else
        {
            // TODO we have a race between create and destroy,
            // but waiting will deadlock: leaking seems the least bad solution
        }
    }

    void resize_titlebar_for(Window const& window, Size const& size)
    {
        if (window.size().width == size.width)
            return;

        if (auto surface = find_titlebar_surface(window))
        {
            SurfaceSpec::for_changes(connection)
                .set_size(size.width.as_int(), title_bar_height)
                .apply_to(surface);
        }
    }

    void advise_new_titlebar(WindowInfo& window_info)
    {
        std::istringstream buffer{window_info.name()};

        void* parent = nullptr;
        buffer >> parent;

        std::lock_guard<decltype(mutex)> lock{mutex};

        for (auto& element : window_to_titlebar)
        {
            auto scene_surface = std::shared_ptr<mir::scene::Surface>(element.first);
            if (scene_surface.get() == parent)
            {
                auto window = window_info.window();
                element.second.window = window;
                auto const& info = tools->info_for(scene_surface);
                window.move_to(info.window().top_left() - Displacement{0, title_bar_height});
                break;
            }
        }
    }

    void move_titlebar_for(miral::WindowInfo const& window_info, Point top_left)
    {
        if (auto window = find_titlebar_window(window_info.window()))
        {
            window.move_to(top_left - Displacement{0, title_bar_height});
        }
    }

    void notify_done()
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        done = true;
        cv.notify_one();
    }

private:
    struct Data
    {
        std::atomic<MirSurface*> titlebar{nullptr};
        Window window;
    };

    static void insert(MirSurface* surface, Data* data)
    {
        data->titlebar = surface;
    }

    using SurfaceMap = std::map<std::weak_ptr<mir::scene::Surface>, Data, std::owner_less<std::weak_ptr<mir::scene::Surface>>>;

    miral::WindowManagerTools* const tools;
    std::mutex mutable mutex;
    MirConnection* connection = nullptr;
    std::weak_ptr<mir::scene::Session> weak_session;

    SurfaceMap window_to_titlebar;
    std::condition_variable cv;
    bool done = false;

    MirSurface* find_titlebar_surface(Window const& window) const
    {
        std::lock_guard<decltype(mutex)> lock{mutex};

        auto const find = window_to_titlebar.find(window);

        return (find != window_to_titlebar.end()) ? find->second.titlebar.load() : nullptr;
    }

    Window find_titlebar_window(Window const& window) const
    {
        std::lock_guard<decltype(mutex)> lock{mutex};

        auto const find = window_to_titlebar.find(window);

        return (find != window_to_titlebar.end()) ? find->second.window : Window{};
    }
};

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
            // TODO this is a rather roundabout way to detect a titlebar
            if (auto const possible_titlebar = tools->window_at(old_cursor))
            {
//                if (auto const parent = tools->info_for(possible_titlebar).parent())
//                {
//                    if (auto const& parent_userdata =
//                        std::static_pointer_cast<TitlebarUserData>(tools->info_for(parent).userdata()))
//                    {
//                        if (possible_titlebar == parent_userdata->window)
//                        {
//                            if (auto const target = tools->window_at(old_cursor))
//                            {
//                                tools->select_active_window(target);
//                                tools->drag_active_window(cursor - old_cursor);
//                            }
//                            consumes_event = true;
//                        }
//                    }
//                }
            }
        }
    }

    old_cursor = cursor;
    return consumes_event;
}


void TitlebarWindowManagerPolicy::advise_new_window(WindowInfo& window_info)
{
    CanonicalWindowManagerPolicy::advise_new_window(window_info);

    if (window_info.window().application() == titlebar_provider->session())
    {
        titlebar_provider->advise_new_titlebar(window_info);
        return;
    }

    if (!window_info.needs_titlebar(window_info.type()))
        return;

    if (window_info.window().application() == spinner.session())
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

//    if (auto const titlebar = std::static_pointer_cast<TitlebarUserData>(window_info.userdata()))
//    {
//        switch (state)
//        {
//        case mir_surface_state_restored:
//            titlebar->window.resize(titlebar_size_for_window(window_info.restore_rect().size));
//            titlebar->window.show();
//            break;
//
//        case mir_surface_state_maximized:
//        case mir_surface_state_vertmaximized:
//        case mir_surface_state_hidden:
//        case mir_surface_state_minimized:
//            titlebar->window.hide();
//            break;
//
//        case mir_surface_state_horizmaximized:
//            titlebar->window.resize(titlebar_size_for_window({display_area.size.width, window_info.restore_rect().size.height}));
//            titlebar->window.show();
//            break;
//
//        case mir_surface_state_fullscreen:
//        default:
//            break;
//        }
//    }
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

void TitlebarWindowManagerPolicy::advise_move_to(miral::WindowInfo const& window_info, Point top_left)
{
    CanonicalWindowManagerPolicy::advise_move_to(window_info, top_left);

    titlebar_provider->move_titlebar_for(window_info, top_left);
}
