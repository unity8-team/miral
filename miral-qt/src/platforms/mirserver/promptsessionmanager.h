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
 */

#ifndef QTMIR_PROMPTSESSIONMANAGER_H
#define QTMIR_PROMPTSESSIONMANAGER_H

#include <memory>

namespace mir {
namespace scene {
class PromptSession;
class PromptSessionManager;
class Session;
}
}

namespace qtmir {
class PromptSessionManager
{
public:
    explicit PromptSessionManager(std::shared_ptr<mir::scene::PromptSessionManager> const& promptSessionManager);
    virtual ~PromptSessionManager();

    std::shared_ptr<mir::scene::Session> application_for(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) const;

    void stop_prompt_session(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) const;
    void suspend_prompt_session(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) const;
    void resume_prompt_session(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) const;

private:
    std::shared_ptr<mir::scene::PromptSessionManager> const m_promptSessionManager;
};
}

#endif //QTMIR_PROMPTSESSIONMANAGER_H
