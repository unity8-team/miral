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
#include <miral/detail/mir_features.h>

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
