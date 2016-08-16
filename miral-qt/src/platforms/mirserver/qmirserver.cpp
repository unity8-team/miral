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

// Qt
#include <QObject>
#include <QCoreApplication>
#include <QDebug>
#include <QOpenGLContext>

// local
#include "argvHelper.h"
#include "mirserver.h"
#include "qmirserver.h"
#include "qmirserver_p.h"
#include "screen.h"
#include "miropenglcontext.h"
#include "usingqtcompositor.h"
#include "logging.h"

// miral
#include <miral/set_terminator.h>

// mir
#include <mir/shell/shell.h>

QMirServer::QMirServer(int &argc, char **argv, QObject *parent)
    : QObject(parent)
    , d_ptr(new QMirServerPrivate())
{
    Q_D(QMirServer);

    d->server = QSharedPointer<MirServer>(new MirServer());

    bool unknownArgsFound = false;
    d->server->set_command_line_handler([&argc, &argv, &unknownArgsFound](int filteredCount, const char* const filteredArgv[]) {
        unknownArgsFound = true;
        // Want to edit argv to match that which Mir returns, as those are for to Qt alone to process. Edit existing
        // argc as filteredArgv only defined in this scope.
        qtmir::editArgvToMatch(argc, argv, filteredCount, filteredArgv);
        });

    // Casting char** to be a const char** safe as Mir won't change it, nor will we
    d->server->set_command_line(argc, const_cast<const char **>(argv));

    usingQtCompositor(*d->server);

    miral::SetTerminator{[](int)
        {
            qDebug() << "Signal caught by Mir, stopping Mir server..";
            QCoreApplication::quit();
        }}(*d->server);

    d->server->add_init_callback([d] {
            d->screensModel->init(d->server->the_display(), d->server->the_compositor(), d->server->the_shell());
        });


    if (!unknownArgsFound) { // mir parsed all the arguments, so edit argv to pretend to have just argv[0]
        argc = 1;
    }

    qCDebug(QTMIR_MIR_MESSAGES) << "MirServer created";
    qCDebug(QTMIR_MIR_MESSAGES) << "Command line arguments passed to Qt:" << QCoreApplication::arguments();

    d->serverThread = new MirServerThread(d);

    connect(d->serverThread, &MirServerThread::stopped, this, &QMirServer::stopped);
}

QMirServer::~QMirServer()
{
    stop();
}

bool QMirServer::start()
{
    Q_D(QMirServer);

    d->serverThread->start(QThread::TimeCriticalPriority);

    if (!d->serverThread->waitForMirStartup())
    {
        qCritical() << "ERROR: QMirServer - Mir failed to start";
        return false;
    }
    d->screensModel->update();
    d->screensController = QSharedPointer<ScreensController>(
                               new ScreensController(d->screensModel, d->server->the_display(),
                                                     d->server->the_display_configuration_controller()));
    Q_EMIT started();
    return true;
}

void QMirServer::stop()
{
    Q_D(QMirServer);

    if (d->serverThread->isRunning()) {
        d->screensController.clear();
        d->serverThread->stop();
        if (!d->serverThread->wait(10000)) {
            // do something to indicate fail during shutdown
            qCritical() << "ERROR: QMirServer - Mir failed to shut down correctly, terminating it";
            d->serverThread->terminate();
        }
    }
}

bool QMirServer::isRunning() const
{
    Q_D(const QMirServer);
    return d->serverThread->isRunning();
}

QWeakPointer<ScreensModel> QMirServer::screensModel() const
{
    Q_D(const QMirServer);
    return d->screensModel;
}

QWeakPointer<ScreensController> QMirServer::screensController() const
{
    Q_D(const QMirServer);
    return d->screensController;
}

QPlatformOpenGLContext *QMirServer::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    Q_D(const QMirServer);
    return new MirOpenGLContext(*d->server->the_display(), *d->server->the_gl_config(), context->format());
}

void *QMirServer::nativeResourceForIntegration(const QByteArray &resource) const
{
    Q_D(const QMirServer);
    void *result = nullptr;

    if (d->server) {
        if (resource == "SessionAuthorizer")
            result = d->m_usingQtMirSessionAuthorizer.the_application_authorizer().get();
        else if (resource == "SessionListener")
            result = d->m_usingQtMirSessionListener.sessionListener();
        else if (resource == "PromptSessionListener")
            result = d->m_usingQtMirPromptSessionListener.promptSessionListener();
        else if (resource == "WindowController")
            result = d->m_usingQtMirWindowManager.windowController();
        else if (resource == "WindowModel")
            result = d->m_usingQtMirWindowManager.windowModel();
        else if (resource == "ScreensController")
            result = screensController().data();
    }
    return result;
}

std::shared_ptr<mir::scene::PromptSessionManager> QMirServer::thePromptSessionManager() const
{
    Q_D(const QMirServer);

    return d->server->the_prompt_session_manager();
}
