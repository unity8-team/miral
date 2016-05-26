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

#ifndef MIRAL_KIOSK_WINDOW_MANAGER_H
#define MIRAL_KIOSK_WINDOW_MANAGER_H

#include "sw_splash.h"

#include "miral/window_management_policy.h"

using namespace mir::geometry;

class KioskWindowManagerPolicy : public miral::WindowManagementPolicy
{
public:
    KioskWindowManagerPolicy(miral::WindowManagerTools* const tools, SwSplash const&);

    void handle_app_info_updated(Rectangles const& displays) override;

    void handle_displays_updated(Rectangles const& displays) override;

    auto handle_place_new_surface(
        miral::ApplicationInfo const& app_info,
        miral::WindowSpecification const& request_parameters)
        -> miral::WindowSpecification override;

    void handle_new_window(miral::WindowInfo& window_info) override;

    void handle_window_ready(miral::WindowInfo& window_info) override;

    void handle_modify_window(miral::WindowInfo& window_info, miral::WindowSpecification const& modifications) override;

    void handle_delete_window(miral::WindowInfo& window_info) override;

    auto handle_set_state(miral::WindowInfo& window_info, MirSurfaceState value) -> MirSurfaceState override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    void handle_raise_window(miral::WindowInfo& window_info) override;

private:
    static const int modifier_mask =
        mir_input_event_modifier_alt |
        mir_input_event_modifier_shift |
        mir_input_event_modifier_sym |
        mir_input_event_modifier_ctrl |
        mir_input_event_modifier_meta;

    auto transform_set_state(miral::WindowInfo& window_info, MirSurfaceState value) -> MirSurfaceState;
    void raise_splash_session() const;

    miral::WindowManagerTools* const tools;

    SwSplash const splash;
};

#endif /* MIRAL_KIOSK_WINDOW_MANAGER_H */
