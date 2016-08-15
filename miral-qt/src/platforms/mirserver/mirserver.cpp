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
#include "screensmodel.h"
#include "logging.h"
#include "usingqtcompositor.h"

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

    usingQtCompositor(*this);

    miral::SetTerminator{[](int)
        {
            qDebug() << "Signal caught by Mir, stopping Mir server..";
            QCoreApplication::quit();
        }}(*this);

    add_init_callback([this, &screensModel] {
        screensModel->init(the_display(), the_compositor(), the_shell());
    });


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

QSharedPointer<ScreensModel> MirServer::screensModel() const
{
    return m_screensModel;
}
