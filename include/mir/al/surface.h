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

#ifndef MIR_ABSTRACTION_SURFACE_H
#define MIR_ABSTRACTION_SURFACE_H

#include "mir/frontend/surface_id.h"

#include <memory>
#include <mir/geometry/size.h>

namespace mir
{
namespace scene { class Session; class Surface; }

namespace al
{
/// Handle class to manage a Mir surface. It may be null (e.g. default initialized) in which case
///
class Surface
{
public:
    Surface();
    Surface(std::shared_ptr <scene::Session> const& session, frontend::SurfaceId surface);
    ~Surface();

    // Indicates that the Surface isn't null
    operator bool() const;

    void set_alpha(float alpha);
    void resize(geometry::Size const& size);
    void show();
    void hide();

    void destroy_surface();

    // TODO remove this conversion which is convenient to maintain stable intermediate forms
    operator std::weak_ptr<scene::Surface>() const;

    // TODO remove this conversion which is convenient to maintain stable intermediate forms
    operator std::shared_ptr<scene::Surface>() const;

private:
    struct Self;
    std::shared_ptr <Self> self;
};
}
}

#endif //MIR_ABSTRACTION_SURFACE_H
