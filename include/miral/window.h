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

#ifndef MIRAL_WINDOW_H
#define MIRAL_WINDOW_H

#include <mir_toolkit/common.h>
#include <mir/geometry/point.h>
#include <mir/geometry/size.h>
#include <mir/geometry/rectangle.h>

// TODO remove frontend::SurfaceId from the interface
#include <mir/frontend/surface_id.h>

#include <memory>
#include <string>
#include <vector>

namespace mir
{
namespace scene { class Session; class Surface; }
namespace shell { class StreamSpecification; }
}

namespace miral
{
class Session;

/// Handle class to manage a Mir surface. It may be null (e.g. default initialized) in which case
///
class Window
{
public:
    Window();
    Window(std::shared_ptr <mir::scene::Session> const& session, mir::frontend::SurfaceId surface);
    ~Window();

    auto type()         const -> MirSurfaceType;
    auto state()        const -> MirSurfaceState;
    auto top_left()     const -> mir::geometry::Point;
    auto size()         const -> mir::geometry::Size;
    auto session()      const -> std::shared_ptr<mir::scene::Session>;
    auto surface_id()   const -> mir::frontend::SurfaceId;
    auto input_bounds() const -> mir::geometry::Rectangle;
    auto input_area_contains(mir::geometry::Point const& point) const -> bool;
    void configure_streams(std::vector<mir::shell::StreamSpecification> const& config);

    // Indicates that the Window isn't null
    operator bool() const;

    void set_alpha(float alpha);
    void resize(mir::geometry::Size const& size);
    void show();
    void hide();
    void rename(std::string const& name);
    void move_to(mir::geometry::Point top_left);
    void set_input_region(std::vector<mir::geometry::Rectangle> const& input_rectangles);
    void set_state(MirSurfaceState state);
    void set_type(MirSurfaceType type);

    void destroy_surface();
    void reset();

    void request_client_surface_close() const;

    // TODO remove this conversion which is convenient to maintain stable intermediate forms
    operator std::weak_ptr<mir::scene::Surface>() const;

    // TODO remove this conversion which is convenient to maintain stable intermediate forms
    operator std::shared_ptr<mir::scene::Surface>() const;

private:
    struct Self;
    std::shared_ptr <Self> self;

    friend bool operator==(Window const& lhs, Window const& rhs);
    friend bool operator==(std::shared_ptr<mir::scene::Surface> const& lhs, Window const& rhs);
    friend bool operator==(Window const& lhs, std::shared_ptr<mir::scene::Surface> const& rhs);
};

bool operator==(Window const& lhs, Window const& rhs);
bool operator==(std::shared_ptr<mir::scene::Surface> const& lhs, Window const& rhs);
bool operator==(Window const& lhs, std::shared_ptr<mir::scene::Surface> const& rhs);

inline bool operator!=(Window const& lhs, Window const& rhs) { return !(lhs == rhs); }
inline bool operator!=(std::shared_ptr<mir::scene::Surface> const& lhs, Window const& rhs) { return !(lhs == rhs); }
inline bool operator!=(Window const& lhs, std::shared_ptr<mir::scene::Surface> const& rhs) { return !(lhs == rhs); }
}

#endif //MIRAL_WINDOW_H
