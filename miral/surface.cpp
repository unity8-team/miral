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

#include "mir/al/basic_window_manager.h"

#include "mir/scene/session.h"
#include "mir/scene/surface.h"

namespace ma = mir::al;

struct ma::Surface::Self
{
    Self(std::shared_ptr<scene::Session> const& session, frontend::SurfaceId surface);

    frontend::SurfaceId const id;
    std::weak_ptr<scene::Session> const session;
    std::weak_ptr<scene::Surface> const surface;
};

ma::Surface::Self::Self(std::shared_ptr<scene::Session> const& session, frontend::SurfaceId surface) :
    id{surface}, session{session}, surface{session->surface(surface)} {}

ma::Surface::Surface(std::shared_ptr<scene::Session> const& session, frontend::SurfaceId surface) :
    self{std::make_shared<Self>(session, surface)}
{
}

auto ma::Surface::surface_id() const -> frontend::SurfaceId
{
    return self->id;
}

void ma::Surface::set_alpha(float alpha)
{
    if (auto const surface = self->surface.lock())
        surface->set_alpha(alpha);
}

ma::Surface::operator std::shared_ptr<scene::Surface>() const
{
    return self->surface.lock();
}

ma::Surface::operator std::weak_ptr<scene::Surface>() const
{
    return self->surface;
}
