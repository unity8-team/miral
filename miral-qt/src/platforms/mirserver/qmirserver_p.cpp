/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "qmirserver_p.h"
#include <QCoreApplication>

// Mir
#include <mir/main_loop.h>

// local
#include "logging.h"
#include "mirdisplayconfigurationpolicy.h"
#include "mirserver.h"
#include "promptsessionlistener.h"
#include "sessionlistener.h"
#include "sessionauthorizer.h"
#include "windowmanagementpolicy.h"
#include "argvHelper.h"
#include "setqtcompositor.h"

// miral
#include <miral/add_init_callback.h>
#include <miral/set_command_line_hander.h>
#include <miral/set_terminator.h>
#include <miral/set_window_managment_policy.h>

void MirServerThread::run()
{
    bool unknownArgsFound = false;

    miral::SetCommandLineHandler setCommandLineHandler{[this, &unknownArgsFound](int filteredCount, const char* const filteredArgv[])
    {
        unknownArgsFound = true;
        // Want to edit argv to match that which Mir returns, as those are for to Qt alone to process. Edit existing
        // argc as filteredArgv only defined in this scope.
        qtmir::editArgvToMatch(argc, argv, filteredCount, filteredArgv);
    }};

    miral::AddInitCallback addInitCallback{[&, this]
    {
        if (!unknownArgsFound) { // mir parsed all the arguments, so edit argv to pretend to have just argv[0]
            argc = 1;
        }
        qCDebug(QTMIR_MIR_MESSAGES) << "MirServer created";
        qCDebug(QTMIR_MIR_MESSAGES) << "Command line arguments passed to Qt:" << QCoreApplication::arguments();

        auto const main_loop = server->server->the_main_loop();
        // By enqueuing the notification code in the main loop, we are
        // ensuring that the server has really and fully started before
        // leaving wait_for_startup().
        main_loop->enqueue(
            this,
            [&]
            {
                std::lock_guard<std::mutex> lock(mutex);
                mir_running = true;
                started_cv.notify_one();
            });
    }};

    miral::SetTerminator setTerminator{[](int)
    {
        qDebug() << "Signal caught by Mir, stopping Mir server..";
        QCoreApplication::quit();
    }};

    qtmir::SetQtCompositor setQtCompositor{server->screensModel};

    // Casting char** to be a const char** safe as Mir won't change it, nor will we
    server->server->set_command_line(argc, const_cast<const char **>(argv));

    // This should eventually be replaced by miral::MirRunner::run_with()
    (*server)(*server->server);
    mir_display_configuration_policy(*server->server);
    setCommandLineHandler(*server->server);
    addInitCallback(*server->server);
    setQtCompositor(*server->server);
    setTerminator(*server->server);

    try {
        server->server->apply_settings();
    } catch (const std::exception &ex) {
        qCritical() << ex.what();
        exit(1);
    }

    server->server->run(); // blocks until Mir server stopped
    Q_EMIT stopped();
}

void MirServerThread::stop()
{
    server->screensModel->terminate();
    server->server->stop();
}

bool MirServerThread::waitForMirStartup()
{
    std::unique_lock<decltype(mutex)> lock(mutex);
    started_cv.wait_for(lock, std::chrono::seconds{10}, [&]{ return mir_running; });
    return mir_running;
}

struct QMirServerPrivate::Self
{
    Self(const QSharedPointer<ScreensModel> &model);
    const QSharedPointer<ScreensModel> &m_screensModel;
    miral::SetWindowManagmentPolicy m_policy;
    qtmir::WindowController m_windowController;
    qtmir::WindowModel m_windowModel;
    std::weak_ptr<SessionListener> m_sessionListener;
    std::weak_ptr<PromptSessionListener> m_promptSessionListener;
};

SessionListener *QMirServerPrivate::sessionListener() const
{
    return self->m_sessionListener.lock().get();
}

QMirServerPrivate::Self::Self(const QSharedPointer<ScreensModel> &model)
    : m_screensModel(model)
    , m_policy(miral::set_window_managment_policy<WindowManagementPolicy>(m_windowModel, m_windowController, m_screensModel))
{
}

qtmir::WindowModelInterface *QMirServerPrivate::windowModel() const
{
    return &self->m_windowModel;
}

qtmir::WindowControllerInterface *QMirServerPrivate::windowController() const
{
    return &self->m_windowController;
}

QMirServerPrivate::QMirServerPrivate() :
    self{std::make_shared<Self>(screensModel)}
{
}

PromptSessionListener *QMirServerPrivate::promptSessionListener() const
{
    return self->m_promptSessionListener.lock().get();
}

void QMirServerPrivate::operator()(mir::Server& server)
{
    qtmir::SetSessionAuthorizer::operator()(server);
    server.override_the_session_listener([this]
        {
            auto const result = std::make_shared<SessionListener>();
            self->m_sessionListener = result;
            return result;
        });

    server.override_the_prompt_session_listener([this]
        {
            auto const result = std::make_shared<PromptSessionListener>();
            self->m_promptSessionListener = result;
            return result;
        });

    self->m_policy(server);
}
