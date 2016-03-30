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

#include "miral/surface.h"

#include "mir/scene/session.h"
#include "mir/scene/surface.h"

struct miral::Surface::Self
{
    Self(std::shared_ptr<mir::scene::Session> const& session, mir::frontend::SurfaceId surface);

    mir::frontend::SurfaceId const id;
    std::weak_ptr<mir::scene::Session> const session;
    std::weak_ptr<mir::scene::Surface> const surface;
};

miral::Surface::Self::Self(std::shared_ptr<mir::scene::Session> const& session, mir::frontend::SurfaceId surface) :
    id{surface}, session{session}, surface{session->surface(surface)} {}

miral::Surface::Surface(std::shared_ptr<mir::scene::Session> const& session, mir::frontend::SurfaceId surface) :
    self{std::make_shared<Self>(session, surface)}
{
}

miral::Surface::Surface()
{
}

miral::Surface::~Surface() = default;

void miral::Surface::set_alpha(float alpha)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->set_alpha(alpha);
}

miral::Surface::operator bool() const
{
    return !!self;
}

miral::Surface::operator std::shared_ptr<mir::scene::Surface>() const
{
    if (!self) return {};
    return self->surface.lock();
}

miral::Surface::operator std::weak_ptr<mir::scene::Surface>() const
{
    if (!self) return {};
    return self->surface;
}

void miral::Surface::resize(mir::geometry::Size const& size)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->resize(size);
}

void miral::Surface::show()
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->show();
}

void miral::Surface::hide()
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->hide();
}

void miral::Surface::destroy_surface()
{
    if (!self) return;
    if (auto const session = self->session.lock())
        session->destroy_surface(self->id);
    self.reset();
}
