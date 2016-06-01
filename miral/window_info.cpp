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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "miral/window_info.h"

#include <mir/geometry/size.h>

#include <limits>

using namespace mir::geometry;

namespace
{
template<typename Value>
auto optional_value_or_default(mir::optional_value<Value> const& optional_value, Value default_ = Value{})
-> Value
{
    return optional_value.is_set() ? optional_value.value() : default_;
}
}

struct miral::WindowInfo::Self
{
    Self(Window window, WindowSpecification const& params);

    Window window;
    MirSurfaceType type;
    MirSurfaceState state;
    mir::geometry::Rectangle restore_rect;
    Window parent;
    std::vector <Window> children;
    mir::geometry::Width min_width;
    mir::geometry::Height min_height;
    mir::geometry::Width max_width;
    mir::geometry::Height max_height;
    mir::optional_value<mir::geometry::DeltaX> width_inc;
    mir::optional_value<mir::geometry::DeltaY> height_inc;
    mir::optional_value<AspectRatio> min_aspect;
    mir::optional_value<AspectRatio> max_aspect;
    mir::optional_value<int> output_id;
    mir::optional_value<MirOrientationMode> preferred_orientation;
    std::shared_ptr<void> userdata;
};

miral::WindowInfo::Self::Self(Window window, WindowSpecification const& params) :
    window{window},
    type{optional_value_or_default(params.type(), mir_surface_type_normal)},
    state{optional_value_or_default(params.state(), mir_surface_state_restored)},
    restore_rect{params.top_left().value(), params.size().value()},
    min_width{optional_value_or_default(params.min_width())},
    min_height{optional_value_or_default(params.min_height())},
    max_width{optional_value_or_default(params.max_width(), Width{std::numeric_limits<int>::max()})},
    max_height{optional_value_or_default(params.max_height(), Height{std::numeric_limits<int>::max()})},
    width_inc{params.width_inc()},
    height_inc{params.height_inc()},
    min_aspect{},
    max_aspect{}
{
    if (min_aspect.is_set())
        min_aspect = AspectRatio{params.min_aspect().value().width, params.min_aspect().value().height};

    if (max_aspect.is_set())
        max_aspect = AspectRatio{params.max_aspect().value().width, params.max_aspect().value().height};

    if (params.output_id().is_set())
        output_id = params.output_id().value();
}

miral::WindowInfo::WindowInfo(
    Window const& window,
    WindowSpecification const& params) :
    self{std::make_unique<Self>(window, params)}
{
    if (params.min_aspect().is_set())
        min_aspect(AspectRatio{params.min_aspect().value().width, params.min_aspect().value().height});

    if (params.max_aspect().is_set())
        max_aspect(AspectRatio{params.max_aspect().value().width, params.max_aspect().value().height});

    if (params.output_id().is_set())
        output_id(params.output_id().value());
}

miral::WindowInfo::~WindowInfo()
{
}

miral::WindowInfo::WindowInfo(WindowInfo const& that) :
    self{std::make_unique<Self>(*that.self)}
{
}

miral::WindowInfo& miral::WindowInfo::operator=(WindowInfo const& that)
{
    *self = *that.self;
    return *this;
}

bool miral::WindowInfo::can_be_active() const
{
    switch (type())
    {
    case mir_surface_type_normal:       /**< AKA "regular"                       */
    case mir_surface_type_utility:      /**< AKA "floating"                      */
    case mir_surface_type_dialog:
    case mir_surface_type_satellite:    /**< AKA "toolbox"/"toolbar"             */
    case mir_surface_type_freestyle:
    case mir_surface_type_menu:
    case mir_surface_type_inputmethod:  /**< AKA "OSK" or handwriting etc.       */
        return true;

    case mir_surface_type_gloss:
    case mir_surface_type_tip:          /**< AKA "tooltip"                       */
    default:
        // Cannot have input focus
        return false;
    }
}

bool miral::WindowInfo::must_have_parent() const
{
    switch (type())
    {
    case mir_surface_type_overlay:;
    case mir_surface_type_inputmethod:
    case mir_surface_type_satellite:
    case mir_surface_type_tip:
        return true;

    default:
        return false;
    }
}

bool miral::WindowInfo::can_morph_to(MirSurfaceType new_type) const
{
    switch (new_type)
    {
    case mir_surface_type_normal:
    case mir_surface_type_utility:
    case mir_surface_type_satellite:
        switch (type())
        {
        case mir_surface_type_normal:
        case mir_surface_type_utility:
        case mir_surface_type_dialog:
        case mir_surface_type_satellite:
            return true;

        default:
            break;
        }
        break;

    case mir_surface_type_dialog:
        switch (type())
        {
        case mir_surface_type_normal:
        case mir_surface_type_utility:
        case mir_surface_type_dialog:
        case mir_surface_type_popover:
        case mir_surface_type_satellite:
            return true;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return false;
}

bool miral::WindowInfo::must_not_have_parent() const
{
    switch (type())
    {
    case mir_surface_type_normal:
    case mir_surface_type_utility:
        return true;

    default:
        return false;
    }
}

bool miral::WindowInfo::is_visible() const
{
    switch (state())
    {
    case mir_surface_state_hidden:
    case mir_surface_state_minimized:
        return false;
    default:
        break;
    }
    return true;
}
void miral::WindowInfo::constrain_resize(Point& requested_pos, Size& requested_size) const
{
    bool const left_resize = requested_pos.x != self->window.top_left().x;
    bool const top_resize  = requested_pos.y != self->window.top_left().y;

    Point new_pos = requested_pos;
    Size new_size = requested_size;

    if (has_min_aspect())
    {
        auto const ar = min_aspect();

        auto const error = new_size.height.as_int()*long(ar.width) - new_size.width.as_int()*long(ar.height);

        if (error > 0)
        {
            // Add (denominator-1) to numerator to ensure rounding up
            auto const width_correction  = (error+(ar.height-1))/ar.height;
            auto const height_correction = (error+(ar.width-1))/ar.width;

            if (width_correction < height_correction)
            {
                new_size.width = new_size.width + DeltaX(width_correction);
            }
            else
            {
                new_size.height = new_size.height - DeltaY(height_correction);
            }
        }
    }

    if (has_max_aspect())
    {
        auto const ar = max_aspect();

        auto const error = new_size.width.as_int()*long(ar.height) - new_size.height.as_int()*long(ar.width);

        if (error > 0)
        {
            // Add (denominator-1) to numerator to ensure rounding up
            auto const height_correction = (error+(ar.width-1))/ar.width;
            auto const width_correction  = (error+(ar.height-1))/ar.height;

            if (width_correction < height_correction)
            {
                new_size.width = new_size.width - DeltaX(width_correction);
            }
            else
            {
                new_size.height = new_size.height + DeltaY(height_correction);
            }
        }
    }

    if (min_width() > new_size.width)
        new_size.width = min_width();

    if (min_height() > new_size.height)
        new_size.height = min_height();

    if (max_width() < new_size.width)
        new_size.width = max_width();

    if (max_height() < new_size.height)
        new_size.height = max_height();

    if (has_width_inc())
    {
        auto const width = new_size.width.as_int() - min_width().as_int();
        auto inc = width_inc().as_int();
        if (width % inc)
            new_size.width = min_width() + DeltaX{inc*(((2L*width + inc)/2)/inc)};
    }

    if (has_height_inc())
    {
        auto const height = new_size.height.as_int() - min_height().as_int();
        auto inc = height_inc().as_int();
        if (height % inc)
            new_size.height = min_height() + DeltaY{inc*(((2L*height + inc)/2)/inc)};
    }

    if (left_resize)
        new_pos.x += new_size.width - requested_size.width;

    if (top_resize)
        new_pos.y += new_size.height - requested_size.height;

    // placeholder - constrain onscreen

    switch (state())
    {
    case mir_surface_state_restored:
        break;

        // "A vertically maximised window is anchored to the top and bottom of
        // the available workspace and can have any width."
    case mir_surface_state_vertmaximized:
        new_pos.y = self->window.top_left().y;
        new_size.height = self->window.size().height;
        break;

        // "A horizontally maximised window is anchored to the left and right of
        // the available workspace and can have any height"
    case mir_surface_state_horizmaximized:
        new_pos.x = self->window.top_left().x;
        new_size.width = self->window.size().width;
        break;

        // "A maximised window is anchored to the top, bottom, left and right of the
        // available workspace. For example, if the launcher is always-visible then
        // the left-edge of the window is anchored to the right-edge of the launcher."
    case mir_surface_state_maximized:
    default:
        new_pos.x = self->window.top_left().x;
        new_pos.y = self->window.top_left().y;
        new_size.width = self->window.size().width;
        new_size.height = self->window.size().height;
        break;
    }

    requested_pos = new_pos;
    requested_size = new_size;
}

bool miral::WindowInfo::needs_titlebar(MirSurfaceType type)
{
    switch (type)
    {
    case mir_surface_type_freestyle:
    case mir_surface_type_menu:
    case mir_surface_type_inputmethod:
    case mir_surface_type_gloss:
    case mir_surface_type_tip:
        // No decorations for these surface types
        return false;
    default:
        return true;
    }
}

auto miral::WindowInfo::type() const -> MirSurfaceType
{
    return self->type;
}

void miral::WindowInfo::type(MirSurfaceType type)
{
    self->type = type;
}

auto miral::WindowInfo::state() const -> MirSurfaceState
{
    return self->state;
}

void miral::WindowInfo::state(MirSurfaceState state)
{
    self->state = state;
}

auto miral::WindowInfo::restore_rect() const -> mir::geometry::Rectangle
{
    return self->restore_rect;
}

void miral::WindowInfo::restore_rect(mir::geometry::Rectangle const& restore_rect)
{
    self->restore_rect = restore_rect;
}

auto miral::WindowInfo::parent() const -> Window
{
    return self->parent;
}

void miral::WindowInfo::parent(Window const& parent)
{
    self->parent = parent;
}

auto miral::WindowInfo::children() const -> std::vector <Window> const&
{
    return self->children;
}

void miral::WindowInfo::add_child(Window const& child)
{
    self->children.push_back(child);
}

void miral::WindowInfo::remove_child(Window const& child)
{
    auto& siblings = self->children;

    for (auto i = begin(siblings); i != end(siblings); ++i)
    {
        if (child == *i)
        {
            siblings.erase(i);
            break;
        }
    }
}

auto miral::WindowInfo::min_width() const -> mir::geometry::Width
{
    return self->min_width;
}

void miral::WindowInfo::min_width(mir::geometry::Width min_width)
{
    self->min_width = min_width;
}

auto miral::WindowInfo::min_height() const -> mir::geometry::Height
{
    return self->min_height;
}

void miral::WindowInfo::min_height(mir::geometry::Height min_height)
{
    self->min_height = min_height;
}

auto miral::WindowInfo::max_width() const -> mir::geometry::Width
{
    return self->max_width;
}

void miral::WindowInfo::max_width(mir::geometry::Width max_width)
{
    self->max_width = max_width;
}

auto miral::WindowInfo::max_height() const -> mir::geometry::Height
{
    return self->max_height;
}

void miral::WindowInfo::max_height(mir::geometry::Height max_height)
{
    self->max_height = max_height;
}

auto miral::WindowInfo::userdata() const -> std::shared_ptr<void>
{
    return self->userdata;
}

void miral::WindowInfo::userdata(std::shared_ptr<void> userdata)
{
    self->userdata = userdata;
}

bool miral::WindowInfo::has_width_inc() const
{
    return self->width_inc.is_set();
}

auto miral::WindowInfo::width_inc() const -> mir::geometry::DeltaX
{
    return self->width_inc.value();
}

void miral::WindowInfo::width_inc(mir::optional_value<mir::geometry::DeltaX> width_inc)
{
    self->width_inc = width_inc;
}

bool miral::WindowInfo::has_height_inc() const
{
    return self->height_inc.is_set();
}

auto miral::WindowInfo::height_inc() const -> mir::geometry::DeltaY
{
    return self->height_inc.value();
}

void miral::WindowInfo::height_inc(mir::optional_value<mir::geometry::DeltaY> height_inc)
{
    self->height_inc = height_inc;
}

bool miral::WindowInfo::has_min_aspect() const
{
    return self->min_aspect.is_set();
}

auto miral::WindowInfo::min_aspect() const -> AspectRatio
{
    return self->min_aspect.value();
}

void miral::WindowInfo::min_aspect(mir::optional_value<AspectRatio> min_aspect)
{
    self->min_aspect = min_aspect;
}

bool miral::WindowInfo::has_max_aspect() const
{
    return self->max_aspect.is_set();
}

auto miral::WindowInfo::max_aspect() const -> AspectRatio
{
    return self->max_aspect.value();
}

void miral::WindowInfo::max_aspect(mir::optional_value<AspectRatio> max_aspect)
{
    self->max_aspect = max_aspect;
}

bool miral::WindowInfo::has_output_id() const
{
    return self->output_id.is_set();
}

auto miral::WindowInfo::output_id() const -> int
{
    return self->output_id.value();
}

void miral::WindowInfo::output_id(mir::optional_value<int> output_id)
{
    self->output_id = output_id;
}

auto miral::WindowInfo::window() const -> Window&
{
    return self->window;
}

bool miral::WindowInfo::has_preferred_orientation() const
{
    return self->preferred_orientation.is_set();
}

auto miral::WindowInfo::preferred_orientation() const -> MirOrientationMode
{
    return self->preferred_orientation.value();
}
void miral::WindowInfo::preferred_orientation(mir::optional_value<MirOrientationMode> preferred_orientation)
{
    self->preferred_orientation = preferred_orientation;
}
