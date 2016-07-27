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

#ifndef MIRAL_SHELL_TITLEBAR_PROVIDER_H
#define MIRAL_SHELL_TITLEBAR_PROVIDER_H


#include <miral/window_manager_tools.h>

#include <mir/geometry/rectangle.h>
#include <mir_toolkit/client_types.h>

#include <atomic>
#include <map>
#include <mutex>
#include <miral/toolkit/connection.h>
#include <thread>
#include <condition_variable>
#include <queue>

class Worker
{
public:
    ~Worker();

    void start_work();
    void enqueue_work(std::function<void()> const& functor);
    void stop_work();

private:
    using WorkQueue = std::queue<std::function<void()>>;

    std::mutex mutable work_mutex;
    std::condition_variable work_cv;
    WorkQueue work_queue;
    bool work_done = false;
    std::thread worker;

    void do_work();
};

class TitlebarProvider : Worker
{
public:
    TitlebarProvider(miral::WindowManagerTools const& tools);
    ~TitlebarProvider();

    void operator()(miral::toolkit::Connection connection);
    void operator()(std::weak_ptr<mir::scene::Session> const& session);

    auto session() const -> std::shared_ptr<mir::scene::Session>;

    void create_titlebar_for(miral::Window const& window);
    void paint_titlebar_for(miral::Window const& window, int intensity);
    void destroy_titlebar_for(miral::Window const& window);
    void resize_titlebar_for(miral::Window const& window, mir::geometry::Size const& size);
    void advise_new_titlebar(miral::WindowInfo& window_info);
    void advise_state_change(miral::WindowInfo const& window_info, MirSurfaceState state);

    void stop();

private:
    struct Data
    {
        std::atomic<MirSurface*> titlebar{nullptr};
        std::function<void(MirSurface* surface)> on_create{[](MirSurface*){}};
        miral::Window window;

        ~Data();
    };

    using SurfaceMap = std::map<std::weak_ptr<mir::scene::Surface>, Data, std::owner_less<std::weak_ptr<mir::scene::Surface>>>;

    miral::WindowManagerTools tools;
    std::mutex mutable mutex;
    miral::toolkit::Connection connection;
    std::weak_ptr<mir::scene::Session> weak_session;

    SurfaceMap window_to_titlebar;

    static void insert(MirSurface* surface, Data* data);
    Data* find_titlebar_data(miral::Window const& window);
    miral::Window find_titlebar_window(miral::Window const& window) const;
};


#endif //MIRAL_SHELL_TITLEBAR_PROVIDER_H
