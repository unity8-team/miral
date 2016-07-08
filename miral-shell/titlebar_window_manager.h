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

#ifndef MIRAL_SHELL_TITLEBAR_WINDOW_MANAGER_H
#define MIRAL_SHELL_TITLEBAR_WINDOW_MANAGER_H

#include "spinner/splash.h"

#include <miral/canonical_window_manager.h>

namespace miral { class InternalClientLauncher; }

using namespace mir::geometry;

class TitlebarProvider;

class TitlebarWindowManagerPolicy : public miral::CanonicalWindowManagerPolicy
{
public:
    TitlebarWindowManagerPolicy(miral::WindowManagerTools* const tools, SpinnerSplash const& spinner, miral::InternalClientLauncher const& launcher);
    ~TitlebarWindowManagerPolicy();

    bool handle_pointer_event(MirPointerEvent const* event) override;
    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    void advise_new_window(miral::WindowInfo& window_info) override;
    void advise_focus_lost(miral::WindowInfo const& info) override;
    void advise_focus_gained(miral::WindowInfo const& info) override;
    void advise_state_change(miral::WindowInfo const& window_info, MirSurfaceState state) override;
    void advise_resize(miral::WindowInfo const& window_info, Size const& new_size) override;
    void advise_delete_window(miral::WindowInfo const& window_info) override;

    void handle_displays_updated(Rectangles const& displays) override;

private:
    miral::WindowManagerTools* const tools;
    SpinnerSplash const spinner;

    std::unique_ptr<TitlebarProvider> const titlebar_provider;

    Rectangle display_area;
    Point old_cursor{};
};

#endif //MIRAL_SHELL_TITLEBAR_WINDOW_MANAGER_H
