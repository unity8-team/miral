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

class MirServer : public QObject,
    public mir::Server
{
    Q_OBJECT

public:
    MirServer(QObject* parent = 0) : QObject(parent)
    {
    }

    ~MirServer() = default;
};

#endif // MIRSERVER_H
