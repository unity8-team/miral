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

#include "mirserverhooks.h"

#include "mircursorimages.h"
#include "mirserverstatuslistener.h"
#include "promptsessionlistener.h"
#include "screenscontroller.h"

// mir
#include <mir/server.h>
#include <mir/graphics/cursor.h>

namespace mg = mir::graphics;

namespace
{
struct HiddenCursorWrapper : mg::Cursor
{
    HiddenCursorWrapper(std::shared_ptr<mg::Cursor> const& wrapped) :
        wrapped{wrapped} { wrapped->hide(); }
    void show() override { }
    void show(mg::CursorImage const&) override { }
    void hide() override { wrapped->hide(); }

    void move_to(mir::geometry::Point position) override { wrapped->move_to(position); }

private:
    std::shared_ptr<mg::Cursor> const wrapped;
};
}

struct qtmir::MirServerHooks::Self
{
    std::weak_ptr<PromptSessionListener> m_promptSessionListener;
    std::weak_ptr<mir::graphics::Display> m_mirDisplay;
    std::weak_ptr<mir::shell::DisplayConfigurationController> m_mirDisplayConfigurationController;
    std::weak_ptr<mir::scene::PromptSessionManager> m_mirPromptSessionManager;
};

qtmir::MirServerHooks::MirServerHooks() :
    self{std::make_shared<Self>()}
{
}

void qtmir::MirServerHooks::operator()(mir::Server& server)
{
    server.override_the_server_status_listener([]
        { return std::make_shared<MirServerStatusListener>(); });

    server.override_the_cursor_images([]
        { return std::make_shared<qtmir::MirCursorImages>(); });

    server.wrap_cursor([&](std::shared_ptr<mg::Cursor> const& wrapped)
        { return std::make_shared<HiddenCursorWrapper>(wrapped); });

    server.override_the_prompt_session_listener([this]
        {
            auto const result = std::make_shared<PromptSessionListener>();
            self->m_promptSessionListener = result;
            return result;
        });

    server.add_init_callback([this, &server]
        {
            self->m_mirDisplay = server.the_display();
            self->m_mirDisplayConfigurationController = server.the_display_configuration_controller();
            self->m_mirPromptSessionManager = server.the_prompt_session_manager();
        });
}

PromptSessionListener *qtmir::MirServerHooks::promptSessionListener() const
{
    if (auto result = self->m_promptSessionListener.lock())
        return result.get();

    throw std::logic_error("No prompt session listener available. Server not running?");
}

std::shared_ptr<mir::scene::PromptSessionManager> qtmir::MirServerHooks::thePromptSessionManager() const
{
    if (auto result = self->m_mirPromptSessionManager.lock())
        return result;

    throw std::logic_error("No prompt session manager available. Server not running?");
}

std::shared_ptr<mir::graphics::Display> qtmir::MirServerHooks::theMirDisplay() const
{
    if (auto result = self->m_mirDisplay.lock())
        return result;

    throw std::logic_error("No display available. Server not running?");
}

QSharedPointer<ScreensController> qtmir::MirServerHooks::createScreensController(QSharedPointer<ScreensModel> const &screensModel) const
{
    return QSharedPointer<ScreensController>(
        new ScreensController(screensModel, theMirDisplay(), self->m_mirDisplayConfigurationController.lock()));
}

