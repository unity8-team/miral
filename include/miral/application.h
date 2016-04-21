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

#ifndef MIRAL_APPLICATION_H
#define MIRAL_APPLICATION_H

#include <memory>

namespace mir
{
namespace scene { class Session; }
}

namespace miral
{
class Window;
class WindowManagerTools;

class Application
{
public:
    explicit Application(WindowManagerTools const* tools, std::weak_ptr<mir::scene::Session> const& scene_session) :
        tools(tools), scene_session{scene_session} {}

    auto kill(int sig) const -> int;

    operator bool() const { return !!scene_session.lock(); }
    operator std::weak_ptr<mir::scene::Session>() const { return scene_session; }
    operator std::shared_ptr<mir::scene::Session>() const { return scene_session.lock(); }

private:
    WindowManagerTools const* tools;
    std::weak_ptr<mir::scene::Session> scene_session;
    friend bool operator==(Application const& lhs, Application const& rhs);
    friend bool operator==(std::shared_ptr<mir::scene::Session> const& lhs, Application const& rhs);
    friend bool operator==(Application const& lhs, std::shared_ptr<mir::scene::Session> const& rhs);
};

bool operator==(Application const& lhs, Application const& rhs);
bool operator==(std::shared_ptr<mir::scene::Session> const& lhs, Application const& rhs);
bool operator==(Application const& lhs, Application const& rhs);

inline bool operator!=(Application const& lhs, Application const& rhs) { return !(lhs == rhs); }
inline bool operator!=(std::shared_ptr<mir::scene::Session> const& lhs, Application const& rhs) { return !(lhs == rhs); }
inline bool operator!=(Application const& lhs, std::shared_ptr<mir::scene::Session> const& rhs) { return !(lhs == rhs); }
}

#endif //MIRAL_APPLICATION_H
