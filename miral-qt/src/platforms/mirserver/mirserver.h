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

class QtEventFeeder;
class MirDisplayConfigurationPolicy;
class ScreensModel;

class MirServer;
class ScreensModel;

// We use virtual inheritance of mir::Server to facilitate derived classes (e.g. testing)
// calling initialization functions before MirServer is constructed.
// TODO Inheriting from mir::Server is a bad idea and leads to this awkward design
// TODO the private UsingQtMirXXX classes will separated out and MirServer will evaporate
class MirServer : public QObject,
    public mir::Server
{
    Q_OBJECT

public:
    MirServer(int &argc, char **argv, const QSharedPointer<ScreensModel> &, QObject* parent = 0);
    ~MirServer() = default;

    void stop();

    QSharedPointer<ScreensModel> screensModel() const;

private:

    const QSharedPointer<ScreensModel> m_screensModel;
};

#endif // MIRSERVER_H
