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

#ifndef MIR_ABSTRACTION_WINDOW_MANAGEMENT_INFO_H
#define MIR_ABSTRACTION_WINDOW_MANAGEMENT_INFO_H

#include "miral/surface.h"

#include <vector>

namespace miral { class Surface; }

namespace mir
{
namespace scene { class Session; class SurfaceCreationParameters; }
namespace al
{
using ::miral::Surface;

struct SessionInfo
{
    std::vector<std::weak_ptr<scene::Surface>> surfaces;

    /// This can be used by client code to store window manager specific information
    std::shared_ptr<void> userdata;
};
}
}

#endif //MIR_ABSTRACTION_WINDOW_MANAGEMENT_INFO_H
