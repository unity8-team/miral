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

#include "promptsessionmanager.h"

#include <mir/scene/prompt_session_manager.h>

qtmir::PromptSessionManager::PromptSessionManager(std::shared_ptr<mir::scene::PromptSessionManager> const& promptSessionManager) :
    m_promptSessionManager{promptSessionManager}
{
}

qtmir::PromptSessionManager::~PromptSessionManager() = default;

std::shared_ptr<mir::scene::Session> qtmir::PromptSessionManager::application_for(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) const
{
    return m_promptSessionManager->application_for(prompt_session);
}

void qtmir::PromptSessionManager::stop_prompt_session(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) const
{
    m_promptSessionManager->stop_prompt_session(prompt_session);
}

void qtmir::PromptSessionManager::suspend_prompt_session(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) const
{
    m_promptSessionManager->suspend_prompt_session(prompt_session);
}

void qtmir::PromptSessionManager::resume_prompt_session(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) const
{
    m_promptSessionManager->resume_prompt_session(prompt_session);
}
