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
#include <QOpenGLContext>

// local
#include "logging.h"
#include "mirdisplayconfigurationpolicy.h"
#include "promptsessionlistener.h"
#include "sessionlistener.h"
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
    auto start_callback = [this]
    {
        std::lock_guard<std::mutex> lock(mutex);
        mir_running = true;
        started_cv.notify_one();
    };

    server->run(start_callback);

    Q_EMIT stopped();
}

bool MirServerThread::waitForMirStartup()
{
    std::unique_lock<decltype(mutex)> lock(mutex);
    started_cv.wait_for(lock, std::chrono::seconds{10}, [&]{ return mir_running; });
    return mir_running;
}

SessionListener *QMirServerPrivate::sessionListener() const
{
    return m_sessionListener.lock().get();
}

qtmir::WindowModelInterface *QMirServerPrivate::windowModel() const
{
    return &m_windowModel;
}

qtmir::WindowControllerInterface *QMirServerPrivate::windowController() const
{
    return &m_windowController;
}

QPlatformOpenGLContext *QMirServerPrivate::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    return m_openGLContextFactory.createPlatformOpenGLContext(context->format(), *m_mirDisplay.lock());
}

std::shared_ptr<mir::scene::PromptSessionManager> QMirServerPrivate::thePromptSessionManager() const
{
    return m_mirPromptSessionManager.lock();
}

QMirServerPrivate::QMirServerPrivate(int argc, char *argv[]) :
    runner(argc, const_cast<const char **>(argv)),
    argc{argc}, argv{argv}
{
}

PromptSessionListener *QMirServerPrivate::promptSessionListener() const
{
    return m_promptSessionListener.lock().get();
}

void QMirServerPrivate::run(std::function<void()> const& start_callback)
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
    }};

    miral::SetTerminator setTerminator{[](int)
    {
        qDebug() << "Signal caught by Mir, stopping Mir server..";
        QCoreApplication::quit();
    }};

    runner.set_exception_handler([this]
    {
        try {
            throw;
        } catch (const std::exception &ex) {
            qCritical() << ex.what();
            exit(1);
        }
    });

    runner.add_start_callback([&]
    {
        screensModel->update();
        screensController = QSharedPointer<ScreensController>(
                                   new ScreensController(screensModel, m_mirDisplay.lock(),
                                                         m_mirDisplayConfigurationController.lock()));
    });

    runner.add_start_callback(start_callback);

    runner.add_stop_callback([&]
    {
        screensModel->terminate();
        screensController.clear();
    });

    runner.run_with(
        {
            m_sessionAuthorizer,
            m_openGLContextFactory,
            [this](mir::Server& ms) { init(ms); },
            miral::set_window_managment_policy<WindowManagementPolicy>(m_windowModel, m_windowController, screensModel),
            qtmir::setDisplayConfigurationPolicy,
            setCommandLineHandler,
            addInitCallback,
            qtmir::SetQtCompositor{screensModel},
            setTerminator,
        });
}

void QMirServerPrivate::stop()
{
    runner.stop();
}

// mir (FIXME)
#include <mir/server.h>

void QMirServerPrivate::init(mir::Server& server)
{
    server.override_the_session_listener([this]
        {
            auto const result = std::make_shared<SessionListener>();
            m_sessionListener = result;
            return result;
        });

    server.override_the_prompt_session_listener([this]
        {
            auto const result = std::make_shared<PromptSessionListener>();
            m_promptSessionListener = result;
            return result;
        });

    server.add_init_callback([this, &server]
        {
            m_mirDisplay = server.the_display();
            m_mirDisplayConfigurationController = server.the_display_configuration_controller();
            m_mirPromptSessionManager = server.the_prompt_session_manager();
        });
}
