/*
 * Copyright © 2017 Canonical Ltd.
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

#ifndef MIRAL_BOTH_VERSIONS_H
#define MIRAL_BOTH_VERSIONS_H

#include <mir/version.h>

#if (MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 26, 0))
    #ifndef __clang__
        #define MIRAL_BOTH_VERSIONS(old_sym, old_version, new_sym, new_version)\
        __asm__(".symver old_sym,old_sym@" #old_version);\
        __asm__(".symver new_sym,new_sym@@" #new_version);
    #else
        #define MIRAL_BOTH_VERSIONS(old_sym, old_version, new_sym, new_version)\
        __asm__(".symver new_sym,old_sym@" #old_version);\
        __asm__(".symver new_sym,new_sym@@@" #new_version);
    #endif
#else
    #ifndef __clang__
        #define MIRAL_BOTH_VERSIONS(old_sym, old_version, new_sym, new_version)\
        extern "C" __attribute__((alias(#old_sym))) void new_sym();\
        __asm__(".symver old_sym,old_sym@" #old_version);\
        __asm__(".symver new_sym,new_sym@@" #new_version);
    #else
#define MIRAL_BOTH_VERSIONS(old_sym, old_version, new_sym, new_version)\
        __asm__(".symver old_sym,old_sym@" #old_version);\
        __asm__(".symver old_sym,new_sym@@@" #new_version);
    #endif
#endif

#endif //MIRAL_BOTH_VERSIONS_H
