/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include "windowmodel.h"

WindowModel::WindowModel()
{

}

WindowModel::~WindowModel()
{

}

void WindowModel::addWindow(const miral::WindowInfo &/*windowInfo*/)
{

}

void WindowModel::removeWindow(const miral::WindowInfo &/*windowInfo*/)
{

}

void WindowModel::focusWindow(const miral::WindowInfo &/*windowInfo*/, bool /*focus*/)
{

}

void WindowModel::moveWindow(miral::WindowInfo &/*windowInfo*/, mir::geometry::Point /*topLeft*/)
{

}

void WindowModel::resizeWindow(miral::WindowInfo &/*windowInfo*/, mir::geometry::Size /*newSize*/)
{

}

void WindowModel::raiseWindows(const std::vector<miral::Window> &/*windows*/)
{

}
