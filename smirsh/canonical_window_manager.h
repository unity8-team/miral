/*
 * Copyright © 2015-2016 Canonical Ltd.
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

#ifndef MIR_EXAMPLE_CANONICAL_WINDOW_MANAGER_H_
#define MIR_EXAMPLE_CANONICAL_WINDOW_MANAGER_H_

#include "miral/window_management_policy.h"
#include "miral/window_manager_tools.h"

#include <mir/geometry/displacement.h>

#include <atomic>
#include <set>

// Based on "Mir and Unity: Surfaces, input, and displays (v0.3)"

namespace mir
{
namespace shell { class DisplayLayout; }
namespace examples
{
using namespace miral;

// standard window management algorithm:
//  o Switch apps: tap or click on the corresponding tile
//  o Move window: Alt-leftmousebutton drag (three finger drag)
//  o Resize window: Alt-middle_button drag (two finger drag)
//  o Maximize/restore current window (to display size): Alt-F11
//  o Maximize/restore current window (to display height): Shift-F11
//  o Maximize/restore current window (to display width): Ctrl-F11
//  o client requests to maximize, vertically maximize & restore
class CanonicalWindowManagerPolicy  : public WindowManagementPolicy
{
public:

    explicit CanonicalWindowManagerPolicy(
        WindowManagerTools* const tools,
        std::shared_ptr<shell::DisplayLayout> const& display_layout);

    void handle_session_info_updated(geometry::Rectangles const& displays) override;

    void handle_displays_updated(geometry::Rectangles const& displays) override;

    auto handle_place_new_surface(
        std::shared_ptr<scene::Session> const& session,
        scene::SurfaceCreationParameters const& request_parameters)
    -> scene::SurfaceCreationParameters override;

    void handle_new_surface(SurfaceInfo& surface_info) override;

    void handle_surface_ready(SurfaceInfo& surface_info) override;

    void handle_modify_surface(SurfaceInfo& surface_info, shell::SurfaceSpecification const& modifications) override;

    void handle_delete_surface(SurfaceInfo& surface_info) override;

    auto handle_set_state(SurfaceInfo& surface_info, MirSurfaceState value) -> MirSurfaceState override;

    bool handle_keyboard_event(MirKeyboardEvent const* event);

    bool handle_touch_event(MirTouchEvent const* event);

    bool handle_pointer_event(MirPointerEvent const* event);

    void handle_raise_surface(SurfaceInfo& surface_info) override;

    void generate_decorations_for(SurfaceInfo& surface_info) override;

private:
    static const int modifier_mask =
        mir_input_event_modifier_alt |
        mir_input_event_modifier_shift |
        mir_input_event_modifier_sym |
        mir_input_event_modifier_ctrl |
        mir_input_event_modifier_meta;

    void drag(geometry::Point cursor);
    void click(geometry::Point cursor);
    void resize(geometry::Point cursor);
    void toggle(MirSurfaceState state);

    // "Mir and Unity: Surfaces, input, and displays (v0.3)" talks about active
    //  *window*,but Mir really only understands surfaces
    void select_active_surface(Surface const& surface);
    auto active_surface() const -> Surface;

    bool resize(Surface const& surface, geometry::Point cursor, geometry::Point old_cursor);
    bool drag(Surface surface, geometry::Point to, geometry::Point from, geometry::Rectangle bounds);
    void move_tree(SurfaceInfo& root, geometry::Displacement movement) const;
    void apply_resize(SurfaceInfo& surface_info, geometry::Point new_pos, geometry::Size new_size) const;

    WindowManagerTools* const tools;
    std::shared_ptr<shell::DisplayLayout> const display_layout;

    geometry::Rectangle display_area;
    geometry::Point old_cursor{};
    Surface active_surface_;
    using FullscreenSurfaces = std::set<Surface>;

    FullscreenSurfaces fullscreen_surfaces;
};
}
}

#endif /* MIR_EXAMPLE_CANONICAL_WINDOW_MANAGER_H_ */
