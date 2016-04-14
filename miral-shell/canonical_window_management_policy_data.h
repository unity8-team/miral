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

#ifndef MIRAL_SHELL_CANONICAL_WINDOW_MANAGEMENT_POLICY_DATA_H
#define MIRAL_SHELL_CANONICAL_WINDOW_MANAGEMENT_POLICY_DATA_H

#include <miral/window.h>

class CanonicalWindowManagementPolicyData
{
public:
    CanonicalWindowManagementPolicyData(miral::Window surface) : surface{surface} { }

    void paint_titlebar(int intensity);

    miral::Window surface;

private:
    struct StreamPainter;
    struct AllocatingPainter;
    struct SwappingPainter;

    std::shared_ptr<StreamPainter> stream_painter;
};

#endif //MIRAL_SHELL_CANONICAL_WINDOW_MANAGEMENT_POLICY_DATA_H
