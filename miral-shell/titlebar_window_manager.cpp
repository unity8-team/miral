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
#include <mir_toolkit/mir_buffer_stream.h>

#include <condition_variable>
#include <map>
#include <mutex>
#include <cstring>
#include <sstream>

using namespace miral;

namespace
{
int const title_bar_height = 10;

void null_bufferstream_callback(MirBufferStream*, void*) {}
void null_surface_callback(MirSurface*, void*) {}
}

using namespace miral::toolkit;

struct TitlebarWindowManagerPolicy::TitlebarProvider
{
    TitlebarProvider(miral::WindowManagerTools* const tools) : tools{tools} {}
    ~TitlebarProvider()
    {
        std::unique_lock<decltype(mutex)> lock{mutex};
        window_to_titlebar.clear();
        done = true;
        cv.notify_one();
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

            for (int j = 0; j != region.height; ++j)
            {
                memset(row, intensity, 4*region.width);
                row += region.stride;
            }

            mir_buffer_stream_swap_buffers(buffer_stream, &null_bufferstream_callback, nullptr);
        }
    }

    void destroy_titlebar_for(Window const& window)
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        window_to_titlebar.erase(window);
    }

    void resize_titlebar_for(Window const& window, Size const& size)
    {
        if (window.size().width == size.width)
            return;

        if (auto titlebar_window = find_titlebar_window(window))
        {
            titlebar_window.resize({size.width, title_bar_height});
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
                auto& parent_info = tools->info_for(scene_surface);
                parent_info.add_child(window);
                window_info.parent(parent_info.window());
                window.move_to(parent_info.window().top_left() - Displacement{0, title_bar_height});
                break;
            }
        }
    }

    void advise_state_change(WindowInfo const& window_info, MirSurfaceState state, Rectangle const& display_area)
    {
        if (auto window = find_titlebar_window(window_info.window()))
        {
            switch (state)
            {
            case mir_surface_state_restored:
                window.resize({window_info.restore_rect().size.width, title_bar_height});
                window.show();
                break;

            case mir_surface_state_maximized:
            case mir_surface_state_vertmaximized:
            case mir_surface_state_hidden:
            case mir_surface_state_minimized:
                window.hide();
                break;

            case mir_surface_state_horizmaximized:
                window.resize({display_area.size.width, title_bar_height});
                window.show();
                break;

            case mir_surface_state_fullscreen:
            default:
                break;
            }
        }
    }

private:
    struct Data
    {
        std::atomic<MirSurface*> titlebar{nullptr};
        Window window;

        ~Data()
        {
            if (auto const surface = titlebar.load())
                mir_surface_release(surface, &null_surface_callback, nullptr);
        }
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
