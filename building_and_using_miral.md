Building and Using MirAL  {#building_and_using_miral}
========================

Getting MirAL
-------------

Depending upon your needs you can build MirAL yourself [Building MirAL].
Alternatively, you can install the MirAL examples [Using MirAL examples] or, 
finally, you can install the headers for using libmiral in development as 
follow:

    $ sudo apt install libmiral-dev
 
Building MirAL
--------------

These instructions assume that you’re using Ubuntu 16.04LTS or later, I’ve not
earlier Ubuntu versions or other distributions.

You’ll need a few development and utility packages installed, along with the
Mir graphics drivers:

    $ sudo apt install devscripts equivs bzr
    $ sudo apt install mir-graphics-drivers-desktop

(If you’re working on a phone or tablet use mir-graphics-drivers-android in
place of mir-graphics-drivers-desktop.)

With these installed you can checkout and build miral:

    $ bzr branch lp:miral
    $ sudo mk-build-deps -i
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

You can install the MirAL examples, headers and libraries you've built with:
  
    $ sudo make install
    
Otherwise, you just need to add "bin/" to the beginning of the commands shown
in the next section. 

### Building the tests

Note that this is not possible for Mir versions prior to 0.24 as there were
bugs in mirtest-dev. At the time of writing Ubuntu 16.04 has Mir-0.21.

Using MirAL examples
--------------------

You can install the MirAL examples as follows:

    $ sudo apt install libmiral-examples

For convenient testing there's a "miral-app" script that wraps the commands used
in the last section to start the server and then launches the gnome-terminal (as
the current user):

    $ miral-app

To run independently of X11 you need to grant access to the graphics hardware
(by running as root) and specify a VT to run in. There's a "miral-desktop"
script that wraps to start the server (as root) and then launch gnome-terminal
(as the current user:

    $ miral-desktop
    
Both the "miral-app" and "miral-desktop" scripts provide options for using an
alternative example shell (miral-kiosk) and an alternative to gnome-terminal.

    -kiosk               use miral-kiosk instead of miral-shell
    -launcher <launcher> use <launcher> instead of 
                         'gnome-terminal --app-id com.canonical.miral.Terminal'

In addition miral-desktop has the option to set the VT that is used:

    -vt       <termid>   set the virtual terminal [4]

There are some additional options (listed with "-h") but those are the important
ones.

Running applications on MirAL
-----------------------------

If you use the command-line launched by miral-app or miral-desktop native Mir
applications (which include native Mir clients and those that use SDL or the 
GTK+, Qt toolkits) can be started as usual:

    $ sudo apt install mir-demos kate neverball 
    $ mir_demo_client_egltriangle
    $ gedit
    $ kate
    $ neverball

From outside the MirAL session GTK+, Qt and SDL applications can still be run
using the miral-run script:

    $ miral-run gedit
    $ miral-run 7kaa

### Running Qt applications

To run Qt applications under Mir you may need to install qtubuntu-desktop:

    $ sudo apt install qtubuntu-desktop

### Running for X11 applications

If you want to run X11 applications that do not have native Mir support in the
toolkit they use then the answer is Xmir: an X11 server that runs on Mir. First
you need Xmir installed:

    $ sudo apt install xmir

Then once you have started a miral shell (as above) you can use miral-xrun to
run applications under Xmir:

    $ miral-xrun firefox

This automatically starts a Xmir X11 server on a new $DISPLAY for the
application to use. You can use miral-xrun both from a command-line outside the
miral-shell or, for example, from the terminal running in the shell.
