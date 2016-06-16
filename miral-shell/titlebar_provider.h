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
#include <condition_variable>

class TitlebarProvider
{
public:
    TitlebarProvider(miral::WindowManagerTools* const tools);
    ~TitlebarProvider();

    void operator()(MirConnection* connection);
    void operator()(std::weak_ptr<mir::scene::Session> const& session);

    auto session() const -> std::shared_ptr<mir::scene::Session>;

    void create_titlebar_for(miral::Window const& window);
    void paint_titlebar_for(miral::Window const& window, int intensity);
    void destroy_titlebar_for(miral::Window const& window);
    void resize_titlebar_for(miral::Window const& window, mir::geometry::Size const& size);
    void advise_new_titlebar(miral::WindowInfo& window_info);
    void advise_state_change(miral::WindowInfo const& window_info, MirSurfaceState state, mir::geometry::Rectangle const& display_area);

private:
    struct Data
    {
        std::atomic<MirSurface*> titlebar{nullptr};
        miral::Window window;

        ~Data();
    };

    using SurfaceMap = std::map<std::weak_ptr<mir::scene::Surface>, Data, std::owner_less<std::weak_ptr<mir::scene::Surface>>>;

    miral::WindowManagerTools* const tools;
    std::mutex mutable mutex;
    MirConnection* connection = nullptr;
    std::weak_ptr<mir::scene::Session> weak_session;

    SurfaceMap window_to_titlebar;
    std::condition_variable cv;
    bool done = false;

    static void insert(MirSurface* surface, Data* data);
    MirSurface* find_titlebar_surface(miral::Window const& window) const;
    miral::Window find_titlebar_window(miral::Window const& window) const;
};


#endif //MIRAL_SHELL_TITLEBAR_PROVIDER_H
