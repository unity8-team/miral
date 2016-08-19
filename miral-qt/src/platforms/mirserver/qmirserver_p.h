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
#include "screenscontroller.h"
#include "windowcontroller.h"
#include "windowmodel.h"
#include "sessionauthorizer.h"

//miral
#include <miral/application_authorizer.h>
#include <miral/runner.h>
#include <miral/set_window_managment_policy.h>

namespace mir { namespace scene { class PromptSessionManager; }}
namespace mir { namespace graphics { class GLConfig; }}
class QMirServer;
class MirServerThread;
class PromptSessionListener;
class SessionListener;
class SessionAuthorizer;
class QPlatformOpenGLContext;
class QOpenGLContext;

namespace qtmir
{
using SetSessionAuthorizer = miral::SetApplicationAuthorizer<SessionAuthorizer>;
}

class QMirServerPrivate : private qtmir::SetSessionAuthorizer
{
public:
    QMirServerPrivate(int argc, char* argv[]);
    const QSharedPointer<ScreensModel> screensModel{new ScreensModel()};
    QSharedPointer<ScreensController> screensController;
    MirServerThread *serverThread;

    void run(std::function<void()> const& start_callback);
    void stop();

    SessionListener *sessionListener() const;
    PromptSessionListener *promptSessionListener() const;
    qtmir::WindowModelInterface *windowModel() const;
    qtmir::WindowControllerInterface *windowController() const;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;
    std::shared_ptr<mir::scene::PromptSessionManager> thePromptSessionManager() const;

    using qtmir::SetSessionAuthorizer::the_application_authorizer;

private:
    void init(mir::Server& server);

    miral::MirRunner runner;
    miral::SetWindowManagmentPolicy m_policy;
    mutable qtmir::WindowController m_windowController;
    mutable qtmir::WindowModel m_windowModel;
    std::weak_ptr<SessionListener> m_sessionListener;
    std::weak_ptr<PromptSessionListener> m_promptSessionListener;
    std::shared_ptr<mir::graphics::Display> m_mirDisplay;
    std::shared_ptr<mir::graphics::GLConfig> m_mirGLConfig;
    std::shared_ptr<mir::shell::DisplayConfigurationController> m_mirDisplayConfigurationController;
    std::shared_ptr<mir::scene::PromptSessionManager> m_mirPromptSessionManager;
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
