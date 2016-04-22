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

#include "miral/window.h"

#include <mir/scene/session.h>
#include <mir/scene/surface.h>
#include <mir/scene/surface_creation_parameters.h>

struct miral::Window::Self
{
    Self(std::shared_ptr<mir::scene::Session> const& session, mir::frontend::SurfaceId surface);

    mir::frontend::SurfaceId const id;
    std::weak_ptr<mir::scene::Session> const session;
    std::weak_ptr<mir::scene::Surface> const surface;
};

miral::Window::Self::Self(std::shared_ptr<mir::scene::Session> const& session, mir::frontend::SurfaceId surface) :
    id{surface}, session{session}, surface{session->surface(surface)} {}

miral::Window::Window(Application const& application, mir::frontend::SurfaceId surface) :
    self{std::make_shared<Self>(application, surface)}
{
}

miral::Window::Window()
{
}

miral::Window::~Window() = default;

void miral::Window::set_alpha(float alpha)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->set_alpha(alpha);
}

miral::Window::operator bool() const
{
    return !!operator std::shared_ptr<mir::scene::Surface>();
}

miral::Window::operator std::shared_ptr<mir::scene::Surface>() const
{
    if (!self) return {};
    return self->surface.lock();
}

miral::Window::operator std::weak_ptr<mir::scene::Surface>() const
{
    if (!self) return {};
    return self->surface;
}

void miral::Window::resize(mir::geometry::Size const& size)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->resize(size);
}

void miral::Window::show()
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->show();
}

void miral::Window::hide()
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->hide();
}

void miral::Window::reset()
{
    self.reset();
}

void miral::Window::set_state(MirSurfaceState state)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->configure(mir_surface_attrib_state, state);
}

void miral::Window::set_type(MirSurfaceType type)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->configure(mir_surface_attrib_type, type);
}

void miral::Window::move_to(mir::geometry::Point top_left)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->move_to(top_left);
}

void miral::Window::request_client_surface_close() const
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->request_client_surface_close();
}

auto miral::Window::top_left() const
-> mir::geometry::Point
{
    if (self)
    {
        if (auto const surface = self->surface.lock())
            return surface->top_left();
    }

    return {};
}

auto miral::Window::size() const
-> mir::geometry::Size
{
    if (self)
    {
        if (auto const surface = self->surface.lock())
            return surface->size();
    }

    return {};
}

auto miral::Window::application() const
-> Application
{
    if (!self) return {};
    return self->session.lock();
}

auto miral::Window::surface_id() const
-> mir::frontend::SurfaceId
{
    if (!self) return {};
    return self->id;
}

auto miral::Window::input_area_contains(mir::geometry::Point const& point) const
-> bool
{
    if (self)
    {
        if (auto const surface = self->surface.lock())
            return surface->input_area_contains(point);
    }

    return false;
}

void miral::Window::configure_streams(std::vector<StreamSpecification> const& config)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
    {
        std::vector<mir::shell::StreamSpecification> dest;
        dest.reserve(config.size());

        for (auto const& stream : config)
            dest.push_back(mir::shell::StreamSpecification{mir::frontend::BufferStreamId{stream.stream_id.as_value()}, stream.displacement});

        application()->configure_streams(*surface, dest);
    }
}

void miral::Window::set_input_region(std::vector<mir::geometry::Rectangle> const& input_rectangles)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->set_input_region(input_rectangles);
}

void miral::Window::rename(std::string const& name)
{
    if (!self) return;
    if (auto const surface = self->surface.lock())
        surface->rename(name);
}

bool miral::operator==(Window const& lhs, Window const& rhs)
{
    return lhs.self == rhs.self;
}

bool miral::operator==(std::shared_ptr<mir::scene::Surface> const& lhs, Window const& rhs)
{
    if (!rhs.self) return !lhs;
    return lhs == rhs.self->surface.lock();
}

bool miral::operator==(Window const& lhs, std::shared_ptr<mir::scene::Surface> const& rhs)
{
    return rhs == lhs;
}
