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

#include "mir/al/surface.h"

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

ma::Surface::Surface()
{
}

ma::Surface::~Surface() = default;

void ma::Surface::set_alpha(float alpha)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->set_alpha(alpha);
}

ma::Surface::operator bool() const
{
    return !!self;
}

ma::Surface::operator std::shared_ptr<scene::Surface>() const
{
    if (!self) return {};
    return self->surface.lock();
}

ma::Surface::operator std::weak_ptr<scene::Surface>() const
{
    if (!self) return {};
    return self->surface;
}

void ma::Surface::resize(geometry::Size const& size)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->resize(size);
}

void ma::Surface::show()
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->show();
}

void ma::Surface::hide()
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->hide();
}

void ma::Surface::destroy_surface()
{
    if (!self) return;
    if (auto const session = self->session.lock())
        session->destroy_surface(self->id);
    self.reset();
}
