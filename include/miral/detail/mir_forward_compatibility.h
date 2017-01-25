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

#ifndef MIRAL_MIR_FORWARD_COMPATIBILITY_H
#define MIRAL_MIR_FORWARD_COMPATIBILITY_H

#include <mir_toolkit/version.h>
#include <mir_toolkit/common.h>
#include <miral/detail/mir_features.h>

#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 5, 0)
using MirWindowAttrib = MirSurfaceAttrib;
auto const mir_window_attrib_type                    = mir_surface_attrib_type;
auto const mir_window_attrib_state                   = mir_surface_attrib_state;
auto const mir_window_attrib_swapinterval            = mir_surface_attrib_swapinterval;
auto const mir_window_attrib_focus                   = mir_surface_attrib_focus;
auto const mir_window_attrib_dpi                     = mir_surface_attrib_dpi;
auto const mir_window_attrib_visibility              = mir_surface_attrib_visibility;
auto const mir_window_attrib_preferred_orientation   = mir_surface_attrib_preferred_orientation;
auto const mir_window_attribs                        = mir_surface_attribs;

using MirWindowType = MirSurfaceType;
auto const mir_window_type_normal        = mir_surface_type_normal;
auto const mir_window_type_utility       = mir_surface_type_utility;
auto const mir_window_type_dialog        = mir_surface_type_dialog;
auto const mir_window_type_gloss         = mir_surface_type_gloss;
auto const mir_window_type_freestyle     = mir_surface_type_freestyle;
auto const mir_window_type_menu          = mir_surface_type_menu;
auto const mir_window_type_inputmethod   = mir_surface_type_inputmethod;
auto const mir_window_type_satellite     = mir_surface_type_satellite;
auto const mir_window_type_tip           = mir_surface_type_tip;
auto const mir_window_types              = mir_surface_types;

using MirWindowState = MirSurfaceState;
auto const mir_window_state_unknown          = mir_surface_state_unknown;
auto const mir_window_state_restored         = mir_surface_state_restored;
auto const mir_window_state_minimized        = mir_surface_state_minimized;
auto const mir_window_state_maximized        = mir_surface_state_maximized;
auto const mir_window_state_vertmaximized    = mir_surface_state_vertmaximized;
auto const mir_window_state_fullscreen       = mir_surface_state_fullscreen;
auto const mir_window_state_horizmaximized   = mir_surface_state_horizmaximized;
auto const mir_window_state_hidden           = mir_surface_state_hidden;
auto const mir_window_states                 = mir_surface_states;

typedef struct MirSurface MirWindow;
typedef struct MirSurfaceParameters MirWindowParameters;
typedef struct MirSurfacePlacementEvent MirWindowPlacementEvent;
typedef struct MirSurfaceSpec MirWindowSpec;
#endif


// Types that don't exist in earlier versions of Mir's toolkit
#if MIR_CLIENT_VERSION < MIR_VERSION_NUMBER(3, 4, 0)

// Inspired by GdkGravity
/**
 * Reference point for aligning a surface relative to a rectangle.
 * Each element (surface and rectangle) has a MirPlacementGravity assigned.
 */
typedef enum MirPlacementGravity
{
    /// the reference point is at the center.
        mir_placement_gravity_center    = 0,

    /// the reference point is at the middle of the left edge.
        mir_placement_gravity_west      = 1 << 0,

    /// the reference point is at the middle of the right edge.
        mir_placement_gravity_east      = 1 << 1,

    /// the reference point is in the middle of the top edge.
        mir_placement_gravity_north     = 1 << 2,

    /// the reference point is at the middle of the lower edge.
        mir_placement_gravity_south     = 1 << 3,

    /// the reference point is at the top left corner.
        mir_placement_gravity_northwest = mir_placement_gravity_north | mir_placement_gravity_west,

    /// the reference point is at the top right corner.
        mir_placement_gravity_northeast = mir_placement_gravity_north | mir_placement_gravity_east,

    /// the reference point is at the lower left corner.
        mir_placement_gravity_southwest = mir_placement_gravity_south | mir_placement_gravity_west,

    /// the reference point is at the lower right corner.
        mir_placement_gravity_southeast = mir_placement_gravity_south | mir_placement_gravity_east
} MirPlacementGravity;

// Inspired by GdkAnchorHints
/**
 * Positioning hints for aligning a window relative to a rectangle.
 *
 * These hints determine how the window should be positioned in the case that
 * the surface would fall off-screen if placed in its ideal position.
 *
 * For example, \p mir_placement_hints_flip_x will invert the x component of
 * \p aux_rect_placement_offset and replace \p mir_placement_gravity_northwest
 * with \p mir_placement_gravity_northeast and vice versa if the window extends
 * beyond the left or right edges of the monitor.
 *
 * If \p mir_placement_hints_slide_x is set, the window can be shifted
 * horizontally to fit on-screen.
 *
 * If \p mir_placement_hints_resize_x is set, the window can be shrunken
 * horizontally to fit.
 *
 * If \p mir_placement_hints_antipodes is set then the rect gravity may be
 * substituted with the opposite corner (e.g. \p mir_placement_gravity_northeast
 * to \p mir_placement_gravity_southwest) in combination with other options.
 *
 * When multiple flags are set, flipping should take precedence over sliding,
 * which should take precedence over resizing.
 */
typedef enum MirPlacementHints
{
    /// allow flipping anchors horizontally
        mir_placement_hints_flip_x   = 1 << 0,

    /// allow flipping anchors vertically
        mir_placement_hints_flip_y   = 1 << 1,

    /// allow sliding window horizontally
        mir_placement_hints_slide_x  = 1 << 2,

    /// allow sliding window vertically
        mir_placement_hints_slide_y  = 1 << 3,

    /// allow resizing window horizontally
        mir_placement_hints_resize_x = 1 << 4,

    /// allow resizing window vertically
        mir_placement_hints_resize_y = 1 << 5,

    /// allow flipping aux_anchor to opposite corner
        mir_placement_hints_antipodes= 1 << 6,

    /// allow flipping anchors on both axes
        mir_placement_hints_flip_any = mir_placement_hints_flip_x|mir_placement_hints_flip_y,

    /// allow sliding window on both axes
        mir_placement_hints_slide_any  = mir_placement_hints_slide_x|mir_placement_hints_slide_y,

    /// allow resizing window on both axes
        mir_placement_hints_resize_any = mir_placement_hints_resize_x|mir_placement_hints_resize_y,
} MirPlacementHints;
#endif

#if !MIRAL_MIR_DEFINES_POINTER_CONFINEMENT
typedef enum MirPointerConfinementState
{
    mir_pointer_unconfined,
    mir_pointer_confined_to_surface,
} MirPointerConfinementState;
#endif

#endif //MIRAL_MIR_FORWARD_COMPATIBILITY_H
