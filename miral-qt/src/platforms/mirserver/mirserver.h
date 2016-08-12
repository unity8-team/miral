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

#ifndef MIRSERVER_H
#define MIRSERVER_H

#include <QObject>
#include <QSharedPointer>
#include <mir/server.h>
#include "miral/set_window_managment_policy.h"
#include "windowmodel.h"
#include "windowcontroller.h"

class QtEventFeeder;
class MirDisplayConfigurationPolicy;
class SessionListener;
class SessionAuthorizer;
class PromptSessionListener;
class ScreensModel;

// TODO the UsingQtMirXXX classes introduced here are a step towards
//      decoupling from libmirserver and using a libmiral-style API
class UsingQtMirSessionAuthorizer
{
public:
    void operator()(mir::Server& server);
    SessionAuthorizer *sessionAuthorizer();

private:
    std::weak_ptr<SessionAuthorizer> m_sessionAuthorizer;
};

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

class MirServer;
class ScreensModel;
class UsingQtMirWindowManager
{
public:
    UsingQtMirWindowManager(const QSharedPointer<ScreensModel> &model);
    void operator()(mir::Server& server);
    qtmir::WindowModelInterface *windowModel();
    qtmir::WindowControllerInterface *windowController();

private:
    const QSharedPointer<ScreensModel> &m_screensModel;
    miral::SetWindowManagmentPolicy m_policy;
    qtmir::WindowController m_windowController;
    qtmir::WindowModel m_windowModel;
};

// We use virtual inheritance of mir::Server to facilitate derived classes (e.g. testing)
// calling initialization functions before MirServer is constructed.
// TODO Inheriting from mir::Server is a bad idea and leads to this awkward design
// TODO the private UsingQtMirXXX classes will separated out and MirServer will evaporate
class MirServer : public QObject,
    private virtual mir::Server,
    private UsingQtMirSessionAuthorizer,
    private UsingQtMirSessionListener,
    private UsingQtMirPromptSessionListener,
    private UsingQtMirWindowManager
{
    Q_OBJECT

    Q_PROPERTY(SessionAuthorizer* sessionAuthorizer READ sessionAuthorizer CONSTANT)
    Q_PROPERTY(SessionListener* sessionListener READ sessionListener CONSTANT)
    Q_PROPERTY(PromptSessionListener* promptSessionListener READ promptSessionListener CONSTANT)

public:
    MirServer(int &argc, char **argv, const QSharedPointer<ScreensModel> &, QObject* parent = 0);
    ~MirServer() = default;

    /* mir specific */
    using mir::Server::run;
    using mir::Server::the_display;
    using mir::Server::the_display_configuration_controller;
    using mir::Server::the_gl_config;
    using mir::Server::the_main_loop;
    using mir::Server::the_prompt_session_manager;

    void stop();

    /* qt specific */
    // getters
    using UsingQtMirSessionAuthorizer::sessionAuthorizer;
    using UsingQtMirSessionListener::sessionListener;
    using UsingQtMirPromptSessionListener::promptSessionListener;
    using UsingQtMirWindowManager::windowModel;
    using UsingQtMirWindowManager::windowController;

    QSharedPointer<ScreensModel> screensModel() const;

private:
    const QSharedPointer<ScreensModel> m_screensModel;
};

#endif // MIRSERVER_H
