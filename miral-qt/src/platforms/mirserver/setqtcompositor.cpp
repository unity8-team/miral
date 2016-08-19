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

#include "setqtcompositor.h"

// local
#include "mircursorimages.h"
#include "mirglconfig.h"
#include "mirserverstatuslistener.h"
#include "qtcompositor.h"
#include "screensmodel.h"

// mir
#include <mir/graphics/cursor.h>
#include <mir/server.h>
#include <mir/shell/shell.h>

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

qtmir::SetQtCompositor::SetQtCompositor(QSharedPointer<ScreensModel> const& screensModel) :
    m_screensModel{screensModel}
{
}

void qtmir::SetQtCompositor::operator()(mir::Server& server)
{
    server.override_the_compositor([this]
    {
        auto result = std::make_shared<QtCompositor>();
        m_compositor = result;
        return result;
    });

    server.override_the_gl_config([]
        { return std::make_shared<MirGLConfig>(); });

    server.override_the_server_status_listener([]
        { return std::make_shared<MirServerStatusListener>(); });

    server.override_the_cursor_images([]
        { return std::make_shared<qtmir::MirCursorImages>(); });

    server.wrap_cursor([&](std::shared_ptr<mg::Cursor> const& wrapped)
        { return std::make_shared<HiddenCursorWrapper>(wrapped); });

    server.add_init_callback([&, this]
        {
            if (auto const compositor = m_compositor.lock())
            {
                m_screensModel->init(server.the_display(), compositor, server.the_shell());
            }
            else
            {
                throw std::logic_error("No m_compositor available. Server not running?");
            }
        });
}

