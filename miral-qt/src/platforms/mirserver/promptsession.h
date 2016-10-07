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

#ifndef QTMIR_PROMPTSESSION_H
#define QTMIR_PROMPTSESSION_H

#include <memory>

namespace mir {
namespace scene {
class PromptSession;
}
}

namespace qtmir {
class PromptSession
{
public:
    // Intentionally promiscuous converting constructor
    PromptSession(std::shared_ptr<mir::scene::PromptSession> const &promptSession) :
        m_promptSession{promptSession} {}

    PromptSession() = default;
    PromptSession(PromptSession const& that) = default;
    PromptSession& operator=(PromptSession const& rhs) = default;
    ~PromptSession() = default;

    mir::scene::PromptSession* get() const { return m_promptSession.get(); }

    friend bool operator==(PromptSession const &lhs, PromptSession const &rhs)
        { return lhs.m_promptSession == rhs.m_promptSession; }

    friend bool operator==(PromptSession const &lhs, mir::scene::PromptSession *rhs)
        { return lhs.m_promptSession.get() == rhs; }

    friend bool operator==(mir::scene::PromptSession *lhs, PromptSession const &rhs)
        { return lhs == rhs.m_promptSession.get(); }

    // Intentionally promiscuous converting operator
    operator std::shared_ptr<mir::scene::PromptSession> const&() const { return m_promptSession; }

private:
    std::shared_ptr<mir::scene::PromptSession> m_promptSession;
};
}

#endif //QTMIR_PROMPTSESSION_H
