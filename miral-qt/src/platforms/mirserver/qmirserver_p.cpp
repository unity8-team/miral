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

// local
#include "logging.h"
#include "mirdisplayconfigurationpolicy.h"
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

// mir (FIXME)
#include <mir/server.h>

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
    }};

    miral::SetTerminator setTerminator{[](int)
    {
        qDebug() << "Signal caught by Mir, stopping Mir server..";
        QCoreApplication::quit();
    }};

    qtmir::SetQtCompositor setQtCompositor{server->screensModel};

    auto initServerPrivate = [this](mir::Server& ms) { server->init(ms); };

    auto& runner = server->runner;

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
        std::lock_guard<std::mutex> lock(mutex);
        mir_running = true;
        started_cv.notify_one();
    });

    runner.add_stop_callback([&]
    {
        server->server = nullptr;
    });

    runner.run_with(
        {
            initServerPrivate,
            qtmir::setDisplayConfigurationPolicy,
            setCommandLineHandler,
            addInitCallback,
            setQtCompositor,
            setTerminator,
        });

    Q_EMIT stopped();
}

void MirServerThread::stop()
{
    server->screensModel->terminate();
    server->runner.stop();
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
    qtmir::WindowModelNotifier m_windowModelNotifier;
    std::weak_ptr<SessionListener> m_sessionListener;
    std::weak_ptr<PromptSessionListener> m_promptSessionListener;
};

SessionListener *QMirServerPrivate::sessionListener() const
{
    return self->m_sessionListener.lock().get();
}

QMirServerPrivate::Self::Self(const QSharedPointer<ScreensModel> &model)
    : m_screensModel(model)
    , m_policy(miral::set_window_managment_policy<WindowManagementPolicy>(m_windowModelNotifier, m_windowController, m_screensModel))
{
}

qtmir::WindowModelNotifierInterface *QMirServerPrivate::windowModelNotifier() const
{
    return &self->m_windowModelNotifier;
}

qtmir::WindowControllerInterface *QMirServerPrivate::windowController() const
{
    return &self->m_windowController;
}

QMirServerPrivate::QMirServerPrivate(int argc, char const* argv[]) :
    runner(argc, argv),
    self{std::make_shared<Self>(screensModel)}
{
}

PromptSessionListener *QMirServerPrivate::promptSessionListener() const
{
    return self->m_promptSessionListener.lock().get();
}

void QMirServerPrivate::init(mir::Server& server)
{
    this->server = &server;

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
