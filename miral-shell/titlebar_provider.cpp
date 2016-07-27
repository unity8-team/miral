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

#include "titlebar_provider.h"

#include <miral/toolkit/surface_spec.h>

#include <mir_toolkit/mir_buffer_stream.h>

#include <cstring>
#include <sstream>

namespace
{
int const title_bar_height = 10;

void null_surface_callback(MirSurface*, void*) {}

void paint_surface(MirSurface* surface, int const intensity)
{
    MirBufferStream* buffer_stream = mir_surface_get_buffer_stream(surface);

    // TODO sometimes buffer_stream is nullptr - find out why (and fix).
    // (Only observed when creating a lot of clients at once)
    if (!buffer_stream)
        return;

    MirGraphicsRegion region;
    mir_buffer_stream_get_graphics_region(buffer_stream, &region);

    char* row = region.vaddr;

    for (int j = 0; j != region.height; ++j)
    {
        memset(row, intensity, 4*region.width);
        row += region.stride;
    }

    mir_buffer_stream_swap_buffers_sync(buffer_stream);
}
}

using namespace miral::toolkit;
using namespace mir::geometry;

TitlebarProvider::TitlebarProvider(miral::WindowManagerTools const& tools) : tools{tools}
{

}

TitlebarProvider::~TitlebarProvider()
{
    stop();
}

void TitlebarProvider::stop()
{
    enqueue_work([this]
        {
            std::lock_guard<decltype(mutex)> lock{mutex};
            window_to_titlebar.clear();
            connection.reset();
            stop_work();
        });
}

void TitlebarProvider::operator()(miral::toolkit::Connection connection)
{
    this->connection = connection;
    start_work();
}

void TitlebarProvider::operator()(std::weak_ptr<mir::scene::Session> const& session)
{
    std::lock_guard<decltype(mutex)> lock{mutex};
    this->weak_session = session;
}

auto TitlebarProvider::session() const -> std::shared_ptr<mir::scene::Session>
{
    std::lock_guard<decltype(mutex)> lock{mutex};
    return weak_session.lock();
}

void TitlebarProvider::create_titlebar_for(miral::Window const& window)
{
    enqueue_work([this, window]
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
        });
}

void TitlebarProvider::paint_titlebar_for(miral::Window const& window, int intensity)
{
    if (auto data = find_titlebar_data(window))
    {
        if (auto surface = data->titlebar.load())
        {
            enqueue_work([this, surface, intensity]{ paint_surface(surface, intensity); });
        }
        else
        {
            data->on_create = [this, intensity](MirSurface* surface)
                { enqueue_work([this, surface, intensity]{ paint_surface(surface, intensity); }); };
        }
    }
}

void TitlebarProvider::destroy_titlebar_for(miral::Window const& window)
{
    if (auto data = find_titlebar_data(window))
    {
        if (auto surface = data->titlebar.exchange(nullptr))
        {
            enqueue_work([surface]
                 {
                     mir_surface_release(surface, &null_surface_callback, nullptr);
                 });
        }

        enqueue_work([this, window]
            {
                std::lock_guard<decltype(mutex)> lock{mutex};
                window_to_titlebar.erase(window);
            });
    }
}

void TitlebarProvider::resize_titlebar_for(miral::Window const& window, Size const& size)
{
    if (window.size().width == size.width)
        return;

    if (auto titlebar_window = find_titlebar_window(window))
    {
        titlebar_window.resize({size.width, title_bar_height});
    }
}

void TitlebarProvider::advise_new_titlebar(miral::WindowInfo& window_info)
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
            auto& parent_info = tools.info_for(scene_surface);
            parent_info.add_child(window);
            window_info.parent(parent_info.window());
            window.move_to(parent_info.window().top_left() - Displacement{0, title_bar_height});
            break;
        }
    }
}

void TitlebarProvider::advise_state_change(miral::WindowInfo const& window_info, MirSurfaceState state)
{
    if (auto titlebar = find_titlebar_window(window_info.window()))
    {
        switch (state)
        {
        case mir_surface_state_maximized:
        case mir_surface_state_vertmaximized:
        case mir_surface_state_hidden:
        case mir_surface_state_minimized:
        case mir_surface_state_fullscreen:
            titlebar.hide();
            break;

        default:
            titlebar.show();
            break;
        }
    }
}

TitlebarProvider::Data::~Data()
{
    if (auto surface = titlebar.exchange(nullptr))
        mir_surface_release(surface, &null_surface_callback, nullptr);
}

void TitlebarProvider::insert(MirSurface* surface, Data* data)
{
    data->on_create(surface);
    data->titlebar = surface;
}

TitlebarProvider::Data* TitlebarProvider::find_titlebar_data(miral::Window const& window)
{
    std::lock_guard<decltype(mutex)> lock{mutex};

    auto const find = window_to_titlebar.find(window);

    return (find != window_to_titlebar.end()) ? &find->second : nullptr;
}

miral::Window TitlebarProvider::find_titlebar_window(miral::Window const& window) const
{
    std::lock_guard<decltype(mutex)> lock{mutex};

    auto const find = window_to_titlebar.find(window);

    return (find != window_to_titlebar.end()) ? find->second.window : miral::Window{};
}

Worker::~Worker()
{
    if (worker.joinable()) worker.join();
}

void Worker::do_work()
{
    while (!work_done)
    {
        WorkQueue::value_type work;
        {
            std::unique_lock<decltype(work_mutex)> lock{work_mutex};
            work_cv.wait(lock, [this] { return !work_queue.empty(); });
            work = work_queue.front();
            work_queue.pop();
        }

        work();
    }
}

void Worker::enqueue_work(std::function<void()> const& functor)
{
    std::lock_guard<decltype(work_mutex)> lock{work_mutex};
    work_queue.push(functor);
    work_cv.notify_one();
}

void Worker::start_work()
{
    worker = std::thread{[this] { do_work(); }};
}

void Worker::stop_work()
{
    enqueue_work([this] { work_done = true; });
}