/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#ifndef QMIRSERVER_P_H
#define QMIRSERVER_P_H

// Qt
#include <QThread>
#include <QSharedPointer>

// std
#include <condition_variable>
#include <mutex>

// local
#include "openglcontextfactory.h"
#include "screensmodel.h"
#include "windowcontroller.h"
#include "windowmodel.h"
#include "sessionauthorizer.h"
#include "mirserverhooks.h"

//miral
#include <miral/application_authorizer.h>
#include <miral/runner.h>

namespace mir { namespace scene { class PromptSessionManager; }}

class QMirServer;
class MirServerThread;
class PromptSessionListener;
class SessionListener;
class SessionAuthorizer;
class QOpenGLContext;

namespace qtmir
{
using SetSessionAuthorizer = miral::SetApplicationAuthorizer<SessionAuthorizer>;
}

class QMirServerPrivate
{
public:
    QMirServerPrivate(int argc, char* argv[]);
    const QSharedPointer<ScreensModel> screensModel{new ScreensModel()};
    QSharedPointer<ScreensController> screensController;
    MirServerThread *serverThread;

    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;

    void run(std::function<void()> const& start_callback);
    void stop();

    SessionListener *sessionListener() const;
    PromptSessionListener *promptSessionListener() const;
    std::shared_ptr<mir::scene::PromptSessionManager> thePromptSessionManager() const;

    auto the_application_authorizer() const -> std::shared_ptr<SessionAuthorizer>
        { return m_sessionAuthorizer.the_custom_application_authorizer(); }

    qtmir::WindowModelInterface *windowModel() const
        { return &m_windowModel; }

    qtmir::WindowControllerInterface *windowController() const
        { return &m_windowController; }

private:
    qtmir::SetSessionAuthorizer m_sessionAuthorizer;
    qtmir::OpenGLContextFactory m_openGLContextFactory;
    qtmir::MirServerHooks       m_mirServerHooks;

    miral::MirRunner runner;

    mutable qtmir::WindowModel      m_windowModel;
    mutable qtmir::WindowController m_windowController;

    int &argc;
    char **argv;
};

class MirServerThread : public QThread
{
    Q_OBJECT

public:
    MirServerThread(QMirServerPrivate* server)
        : server(server)
    {}

    bool waitForMirStartup();

Q_SIGNALS:
    void stopped();

public Q_SLOTS:
    void run() override;

private:
    std::mutex mutex;
    std::condition_variable started_cv;
    bool mir_running{false};

    QMirServerPrivate* const server;
};

#endif // QMIRSERVER_P_H
