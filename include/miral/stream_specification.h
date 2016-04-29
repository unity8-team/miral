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

#ifndef MIRAL_STREAM_SPECIFICATION_H
#define MIRAL_STREAM_SPECIFICATION_H

#include <mir/int_wrapper.h>
#include <mir/geometry/displacement.h>
#include <mir/geometry/size.h>
#include <mir/optional_value.h>

namespace miral
{
using namespace mir::geometry;

namespace detail { struct SessionsBufferStreamIdTag; }
typedef mir::IntWrapper<detail::SessionsBufferStreamIdTag> BufferStreamId;

struct StreamSpecification
{
    BufferStreamId stream_id;
    Displacement displacement;
    mir::optional_value<mir::geometry::Size> size;
};
}

#endif //MIRAL_STREAM_SPECIFICATION_H
