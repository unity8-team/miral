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
#include "miral/set_window_managment_policy.h"

class MirServer;
class QMirServer;
class MirServerThread;
class PromptSessionListener;
class SessionListener;
class SessionAuthorizer;

// TODO the UsingQtMirXXX classes introduced here are a step towards
//      decoupling from libmirserver and using a libmiral-style API
using UsingQtMirSessionAuthorizer = miral::SetApplicationAuthorizer<SessionAuthorizer>;

class UsingQtMirSessionListener
{
public:
    void operator()(mir::Server& server);
    SessionListener *sessionListener();

private:
    std::weak_ptr<SessionListener> m_sessionListener;
};

class UsingQtMirPromptSessionListener
{
public:
    void operator()(mir::Server& server);
    PromptSessionListener *promptSessionListener();

private:
    std::weak_ptr<PromptSessionListener> m_promptSessionListener;
};

class UsingQtMirWindowManager
{
public:
    UsingQtMirWindowManager(const QSharedPointer<ScreensModel> &model);
    void operator()(mir::Server& server);
    qtmir::WindowModelNotifierInterface *windowModel();
    qtmir::WindowControllerInterface *windowController();

private:
    const QSharedPointer<ScreensModel> &m_screensModel;
    miral::SetWindowManagmentPolicy m_policy;
    qtmir::WindowController m_windowController;
    qtmir::WindowModel m_windowModel;
};

class QMirServerPrivate
{
public:
    QSharedPointer<ScreensModel> screensModel{new ScreensModel()};
    QSharedPointer<MirServer> server;
    QSharedPointer<ScreensController> screensController;
    MirServerThread *serverThread;

    UsingQtMirSessionAuthorizer m_usingQtMirSessionAuthorizer;
    UsingQtMirSessionListener mutable m_usingQtMirSessionListener;
    UsingQtMirPromptSessionListener mutable m_usingQtMirPromptSessionListener;
    UsingQtMirWindowManager mutable m_usingQtMirWindowManager{screensModel};
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
    void stop();

private:
    std::mutex mutex;
    std::condition_variable started_cv;
    bool mir_running{false};

    QMirServerPrivate* const server;
};

#endif // QMIRSERVER_P_H
