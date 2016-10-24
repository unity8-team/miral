Building and using MirAL  {#building_and_using_miral}
========================

These instructions assume that you’re using Ubuntu 16.04LTS or later, I’ve not
earlier Ubuntu versions or other distributions.

You’ll need a few development and utility packages installed, along with the
Mir graphics drivers:

    $ sudo apt-get install devscripts equivs bzr
    $ sudo apt-get install mir-graphics-drivers-desktop

(If you’re working on a phone or tablet use mir-graphics-drivers-android in
place of mir-graphics-drivers-desktop.)

With these installed you can checkout and build miral:

    $ bzr branch lp:miral
    $ sudo mk-build-deps -i --build-dep miral/debian/control
    $ mkdir miral/build
    $ cd  miral/build
    $ cmake ..
    $ make
    
This creates libmiral.so in the lib directory and an example shell 
(miral-shell) in the bin directory. This can be run directly:

    $ bin/miral-shell
    
With the default options this runs in a window on X (which is convenient for
development). 

The miral-shell example is simple, don’t expect to see a sophisticated launcher
by default. You can start mir apps from the command-line. For example:
 
    $ bin/miral-run gnome-terminal
    
That’s right, a lot of standard GTK+ applications will “just work” (the GDK
toolkit has a Mir backend). Any that assume the existence of an X11 and bypass
the toolkit my making X11 protocol calls will have problems though.

To exit from miral-shell press Ctrl-Alt-BkSp.

To run independently of X11 you need to grant access to the graphics hardware
(by running as root) and specify a VT to run in. For example:

    $ sudo bin/miral-shell --vt 4 --arw-file --file $XDG_RUNTIME_DIR/mir_socket
    
For convenient testing there's a "testrun" script that wraps this command to
start the server (as root) and then launches the gnome-terminal (as the current
user):
 
    $ ../scripts/testrun

Support for X11 applications
----------------------------

If you want to run X11 applications that do not have native Mir support in the
toolkit they use then the answer is Xmir: an X11 server that runs on Mir. First
you need Xmir installed:
 
    $ sudo apt install xmir

Then once you have started miral-shell (as above) you can use miral-xrun to run
applications under Xmir:

    $ bin/miral-xrun firefox

This automatically starts a Xmir X11 server on a new $DISPLAY for the 
application to use. You can use miral-xrun both from a command-line outside the
miral-shell or, for example, from a terminal running in the shell.

Running applications on Miral
-----------------------------

Assuming you have a Mir server running, native Mir applications can be started
from the command-line:

    $ sudo apt-get install mir-demos
    $ mir_demo_client_egltriangle

Similarly, GTK+, Qt and SDL applications can be run with the miral-run script:
 
    $ bin/miral-run gedit
    $ bin/miral-run 7kaa

Configuration options
---------------------

You can list the configuration options for miral-shell with "--help":

    $ bin/miral-shell --help
    
Most of these options are inherited from Mir. These can be set on the command
line, by environment variables or in a config file. For example, if you want to
start the gnome-terminal when you run miral-shell you can:

Set supply the option on the command line:

    $ bin/miral-shell --startup-apps gnome-terminal
    
Set the corresponding MIR_SERVER_<option> environment variable:
    
    $ export MIR_SERVER_STARTUP_APPS=gnome-terminal
    ...
    $ bin/miral-shell
    
Create a miral-shell config file:

    $ echo startup-apps=gnome-terminal > ~/.config/miral-shell.config
    ...
    $ bin/miral-shell

Running Qt applications
-----------------------

To run Qt applications under Mir you may need to install qtubuntu-desktop:

    $ sudo apt-get install qtubuntu-desktop

Building the tests
------------------

MirAL comes with a set of tests, if you intend to build these then add the
following packages:

    $ sudo apt-get install cmake-extras google-mock mirtest-dev mir-test-tools
    $ sudo apt-get install libboost-filesystem-dev libboost-system-dev

Note that this is not possible for Mir versions prior to 0.24 as there were
bugs in mirtest-dev. At the time of writing Ubuntu 16.4 has Mir-0.21.
