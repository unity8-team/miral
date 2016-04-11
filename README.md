Readme
======

MirAL - The Mir Abstraction Layer
---------------------------------

MirAL aims to provide a stable “abstraction layer” written over the top of the
Mir server API that will provide a stable ABI. There are a number of other 
goals that can be addressed at the same time:

 - The API can be considerably narrowed as a lot of things can be customized 
   that are of no interest to shell development;
 - A more declarative design style can be follow than the implementation 
   focussed approach that the Mir server API follows; and,
 - Common facilities that don’t belong in the Mir libraries can be provided.

The Mir Abstraction Layer (miral) is a both a proof-of-concept and a 
work-in-progress but may be of interest as modern C++ codebase to experiment
with.

There are two significant components to the MirAL project:

 - libmiral.so is the abstraction library itself
 - miral-shell is an example shell demonstrating the use of libmiral

See also:

 - \ref building_and_using_miral "Building and using Miral"
 - \ref tasks_for_the_interested_reader "Tasks for the interested reader"

License
-------
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

\copyright Copyright © 2016 Canonical Ltd.
\author Alan Griffiths <alan@octopull.co.uk>
