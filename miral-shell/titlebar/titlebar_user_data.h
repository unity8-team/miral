/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIRAL_SHELL_TITLEBAR_USER_DATA_H
#define MIRAL_SHELL_TITLEBAR_USER_DATA_H

#include <miral/window.h>

class TitlebarUserData
{
public:
    TitlebarUserData(miral::Window window) : window{window} { }

    void paint_titlebar(int intensity);

    miral::Window window;

private:
    struct StreamPainter;
    struct AllocatingPainter;
    struct SwappingPainter;

    std::shared_ptr<StreamPainter> stream_painter;
};

#endif //MIRAL_SHELL_TITLEBAR_USER_DATA_H
