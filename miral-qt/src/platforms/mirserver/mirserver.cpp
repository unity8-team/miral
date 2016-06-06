/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>

#include "mirserver.h"
#include "basic_window_manager.h"

// local
#include "argvHelper.h"
#include "mircursorimages.h"
#include "mirdisplayconfigurationpolicy.h"
#include "mirglconfig.h"
#include "mirserverstatuslistener.h"
#include "mirwindowmanager.h"
#include "promptsessionlistener.h"
#include "screensmodel.h"
#include "sessionlistener.h"
#include "sessionauthorizer.h"
#include "qtcompositor.h"
#include "windowmanagementpolicy.h"
#include "logging.h"

// std
#include <memory>

// egl
#define MESA_EGL_NO_X11_HEADERS
#include <EGL/egl.h>

// mir
#include <mir/graphics/cursor.h>

namespace mg = mir::graphics;
namespace mo  = mir::options;
namespace msh = mir::shell;
namespace ms = mir::scene;

namespace
{
void usingHiddenCursor(mir::Server& server);
}

MirServer::MirServer(int &argc, char **argv,
                     const QSharedPointer<ScreensModel> &screensModel, QObject* parent)
    : QObject(parent)
    , m_screensModel(screensModel)
{
    bool unknownArgsFound = false;
    set_command_line_handler([&argc, &argv, &unknownArgsFound](int filteredCount, const char* const filteredArgv[]) {
        unknownArgsFound = true;
        // Want to edit argv to match that which Mir returns, as those are for to Qt alone to process. Edit existing
        // argc as filteredArgv only defined in this scope.
        qtmir::editArgvToMatch(argc, argv, filteredCount, filteredArgv);
    });

    // Casting char** to be a const char** safe as Mir won't change it, nor will we
    set_command_line(argc, const_cast<const char **>(argv));

    UsingQtMirSessionAuthorizer::operator()(*this);
    UsingQtMirSessionListener::operator()(*this);
    UsingQtMirPromptSessionListener::operator()(*this);
    UsingQtMirWindowManager::operator()(*this);

    override_the_session_authorizer([]
        {
            return std::make_shared<SessionAuthorizer>();
        });

    override_the_compositor([]
        {
            return std::make_shared<QtCompositor>();
        });

    override_the_cursor_images([]
        {
            return std::make_shared<qtmir::MirCursorImages>();
        });

    override_the_gl_config([]
        {
            return std::make_shared<MirGLConfig>();
        });

    override_the_server_status_listener([]
        {
            return std::make_shared<MirServerStatusListener>();
        });

    wrap_display_configuration_policy(
        [](const std::shared_ptr<mg::DisplayConfigurationPolicy> &wrapped)
            -> std::shared_ptr<mg::DisplayConfigurationPolicy>
        {
            return std::make_shared<MirDisplayConfigurationPolicy>(wrapped);
        });

    set_terminator([](int)
        {
            qDebug() << "Signal caught by Mir, stopping Mir server..";
            QCoreApplication::quit();
        });

    add_init_callback([this, &screensModel] {
        screensModel->init(the_display(), the_compositor());
    });

    usingHiddenCursor(*this);

    try {
        apply_settings();
    } catch (const std::exception &ex) {
        qCritical() << ex.what();
        exit(1);
    }

    if (!unknownArgsFound) { // mir parsed all the arguments, so edit argv to pretend to have just argv[0]
        argc = 1;
    }

    qCDebug(QTMIR_MIR_MESSAGES) << "MirServer created";
    qCDebug(QTMIR_MIR_MESSAGES) << "Command line arguments passed to Qt:" << QCoreApplication::arguments();
}

// Override default implementation to ensure we terminate the ScreensModel first.
// Code path followed when Qt tries to shutdown the server.
void MirServer::stop()
{
    m_screensModel->terminate();
    mir::Server::stop();
}


/************************************ Shell side ************************************/

void UsingQtMirSessionAuthorizer::operator()(mir::Server& server)
{
    server.override_the_session_authorizer([this]
        {
            auto const result = std::make_shared<SessionAuthorizer>();
            m_sessionAuthorizer = result;
            return result;
        });
}

SessionAuthorizer *UsingQtMirSessionAuthorizer::sessionAuthorizer()
{
    return m_sessionAuthorizer.lock().get();
}

void UsingQtMirSessionListener::operator()(mir::Server& server)
{
    server.override_the_session_listener([this]
        {
            auto const result = std::make_shared<SessionListener>();
            m_sessionListener = result;
            return result;
        });
}

SessionListener *UsingQtMirSessionListener::sessionListener()
{
    return m_sessionListener.lock().get();
}

void UsingQtMirPromptSessionListener::operator()(mir::Server& server)
{
    server.override_the_prompt_session_listener([this]
        {
            auto const result = std::make_shared<PromptSessionListener>();
            m_promptSessionListener = result;
            return result;
        });
}

PromptSessionListener *UsingQtMirPromptSessionListener::promptSessionListener()
{
    return m_promptSessionListener.lock().get();
}

void UsingQtMirWindowManager::operator()(MirServer& server)
{
    server.override_the_window_manager_builder([this,&server](msh::FocusController* focus_controller)
        -> std::shared_ptr<msh::WindowManager>
        {
            auto const display_layout = server.the_shell_display_layout();
            auto const policy = [&server](miral::WindowManagerTools* tools){
                return std::make_unique<WindowManagementPolicy>(tools, server.screensModel());
            };

            return std::make_shared<miral::BasicWindowManager>(focus_controller, display_layout, policy);
        });
//    server.override_the_window_manager_builder([this,&server](mir::shell::FocusController*)
//        -> std::shared_ptr<mir::shell::WindowManager>
//        {
//            auto windowManager = MirWindowManager::create(server.the_shell_display_layout(),
//                    std::static_pointer_cast<::SessionListener>(server.the_session_listener()));
//            m_windowManager = windowManager;
//            return windowManager;
//        });
}

MirWindowManager *UsingQtMirWindowManager::windowManager()
{
    return m_windowManager.lock().get();
}

mir::shell::Shell *MirServer::shell()
{
    std::weak_ptr<mir::shell::Shell> m_shell = the_shell();
    return m_shell.lock().get();
}

QSharedPointer<ScreensModel> MirServer::screensModel() const
{
    return m_screensModel;
}

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

void usingHiddenCursor(mir::Server& server)
{
    server.wrap_cursor([&](std::shared_ptr<mg::Cursor> const& wrapped)
        { return std::make_shared<HiddenCursorWrapper>(wrapped); });
}
}
