/*
 * Copyright © 2014-2016 Canonical Ltd.
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

#include "window_management.h"

#include "tiling_window_manager.h"
#include "canonical_window_manager.h"

#include "mir/abnormal_exit.h"
#include "mir/server.h"
#include "mir/input/composite_event_filter.h"
#include "mir/options/option.h"
#include "mir/scene/session.h"
#include "mir/scene/surface_creation_parameters.h"
#include "mir/shell/display_layout.h"
#include "mir/shell/system_compositor_window_manager.h"

namespace me = mir::examples;
namespace mf = mir::frontend;
namespace mg = mir::graphics;
namespace mi = mir::input;
namespace ms = mir::scene;
namespace msh = mir::shell;
using namespace mir::geometry;

// Demonstrate introducing a window management strategy
namespace
{
char const* const wm_option = "window-manager";
char const* const wm_description = "window management strategy [{tiling|fullscreen|canonical|system-compositor}]";

char const* const wm_tiling = "tiling";
char const* const wm_fullscreen = "fullscreen";
char const* const wm_canonical = "canonical";
char const* const wm_system_compositor = "system-compositor";

// Very simple - make every surface fullscreen
class FullscreenWindowManagerPolicy  : public me::WindowManagementPolicy
{
public:
    FullscreenWindowManagerPolicy(me::WindowManagerTools* const /*tools*/, std::shared_ptr<msh::DisplayLayout> const& display_layout) :
        display_layout{display_layout} {}

    void handle_session_info_updated(SessionInfoMap& /*session_info*/, Rectangles const& /*displays*/) {}

    void handle_displays_updated(SessionInfoMap& /*session_info*/, Rectangles const& /*displays*/) {}

    auto handle_place_new_surface(
        std::shared_ptr<ms::Session> const& /*session*/,
        ms::SurfaceCreationParameters const& request_parameters)
    -> ms::SurfaceCreationParameters
    {
        auto placed_parameters = request_parameters;

        Rectangle rect{request_parameters.top_left, request_parameters.size};
        display_layout->size_to_output(rect);
        placed_parameters.size = rect.size;

        return placed_parameters;
    }
    void handle_modify_surface(mir::al::SurfaceInfo&, msh::SurfaceSpecification const&) override
    {
    }

    void handle_new_surface(mir::al::SurfaceInfo& /*surface_info*/) override
    {
    }

    void handle_delete_surface(mir::al::SurfaceInfo& surface_info) override
        { surface_info.surface.destroy_surface(); }

    int handle_set_state(mir::al::SurfaceInfo& /*surface_info*/, MirSurfaceState value)
        { return value; }

    bool handle_keyboard_event(MirKeyboardEvent const* /*event*/) { return false; }

    bool handle_touch_event(MirTouchEvent const* /*event*/) { return false; }

    bool handle_pointer_event(MirPointerEvent const* /*event*/) { return false; }

    void handle_raise_surface(
        std::shared_ptr<ms::Session> const& /*session*/,
        std::shared_ptr<ms::Surface> const& /*surface*/)
    {
    }

    void generate_decorations_for(mir::al::SurfaceInfo& /*surface_info*/) override
    {
    }
private:
    std::shared_ptr<msh::DisplayLayout> const display_layout;
};
}

using FullscreenWindowManager = me::WindowManagerBuilder<FullscreenWindowManagerPolicy>;
using CanonicalWindowManager = me::WindowManagerBuilder<me::CanonicalWindowManagerPolicy>;

void me::window_manager_option(Server& server)
{
    server.add_configuration_option(wm_option, wm_description, wm_canonical);

    server.override_the_window_manager_builder([&server](msh::FocusController* focus_controller)
        -> std::shared_ptr<msh::WindowManager>
        {
            auto const options = server.get_options();
            auto const selection = options->get<std::string>(wm_option);

            if (selection == wm_tiling)
            {
                return std::make_shared<TilingWindowManager>(focus_controller);
            }
            else if (selection == wm_fullscreen)
            {
                return std::make_shared<FullscreenWindowManager>(focus_controller, server.the_shell_display_layout());
            }
            else if (selection == wm_canonical)
            {
                return std::make_shared<CanonicalWindowManager>(focus_controller, server.the_shell_display_layout());
            }
            else if (selection == wm_system_compositor)
            {
                return std::make_shared<msh::SystemCompositorWindowManager>(
                    focus_controller,
                    server.the_shell_display_layout(),
                    server.the_session_coordinator());
            }

            throw mir::AbnormalExit("Unknown window manager: " + selection);
        });
}
