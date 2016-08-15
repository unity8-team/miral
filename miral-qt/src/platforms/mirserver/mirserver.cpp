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

// local
#include "argvHelper.h"
#include "mirdisplayconfigurationpolicy.h"
#include "promptsessionlistener.h"
#include "screensmodel.h"
#include "sessionlistener.h"
#include "sessionauthorizer.h"
#include "windowmanagementpolicy.h"
#include "logging.h"
#include "using_qt_compositor.h"

// std
#include <memory>

// egl
#define MESA_EGL_NO_X11_HEADERS
#include <EGL/egl.h>

// mir
#include <mir/shell/shell.h>

// miral
#include <miral/set_terminator.h>

namespace mg = mir::graphics;
namespace mo  = mir::options;
namespace msh = mir::shell;
namespace ms = mir::scene;

MirServer::MirServer(int &argc, char **argv,
                     const QSharedPointer<ScreensModel> &screensModel, QObject* parent)
    : QObject(parent)
    , UsingQtMirWindowManager(screensModel)
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

    usingQtCompositor(*this);

    wrap_display_configuration_policy(
        [](const std::shared_ptr<mg::DisplayConfigurationPolicy> &wrapped)
            -> std::shared_ptr<mg::DisplayConfigurationPolicy>
        {
            return std::make_shared<MirDisplayConfigurationPolicy>(wrapped);
        });


    miral::SetTerminator{[](int)
        {
            qDebug() << "Signal caught by Mir, stopping Mir server..";
            QCoreApplication::quit();
        }}(*this);

    add_init_callback([this, &screensModel] {
        screensModel->init(the_display(), the_compositor(), the_shell());
    });

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

UsingQtMirWindowManager::UsingQtMirWindowManager(const QSharedPointer<ScreensModel> &model)
    : m_screensModel(model)
    , m_policy(miral::set_window_managment_policy<WindowManagementPolicy>(m_windowModel, m_windowController, m_screensModel))
{
}

void UsingQtMirWindowManager::operator()(mir::Server& server)
{
    m_policy(server);
}

qtmir::WindowModelInterface *UsingQtMirWindowManager::windowModel()
{
    return &m_windowModel;
}

qtmir::WindowControllerInterface *UsingQtMirWindowManager::windowController()
{
    return &m_windowController;
}

QSharedPointer<ScreensModel> MirServer::screensModel() const
{
    return m_screensModel;
}
