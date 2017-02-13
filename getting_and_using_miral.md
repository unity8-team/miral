Getting and Using MirAL  {#getting_and_using_miral}
=======================

Getting MirAL
-------------

Depending upon your needs you can:
 
1. Install and use the MirAL examples [Using MirAL examples];
2. Use MirAL to develop your own Mir server [Using MirAL for development]; or,
3. Build MirAL yourself [Building MirAL]. 
 
Using MirAL examples
--------------------

You can install the MirAL examples as follows:

    $ sudo apt install libmiral-examples
    $ sudo apt install mir-graphics-drivers-desktop qtubuntu-desktop

For convenient testing there's a "miral-app" script that wraps the commands used
in the last section to start the server and then launches the gnome-terminal (as
the current user):

    $ miral-app

To run independently of X11 you need to grant access to the graphics hardware
(by running as root) and specify a VT to run in. There's a "miral-desktop"
script that wraps to start the server (as root) and then launch gnome-terminal
(as the current user):

    $ miral-desktop
    
For more options see [Options when running the MirAL example shell] below.
    
### Running applications on MirAL

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

### Options when running the MirAL example shell

#### Script Options

Both the "miral-app" and "miral-desktop" scripts provide options for using an
alternative example shell (miral-kiosk) and an alternative to gnome-terminal.

    -kiosk               use miral-kiosk instead of miral-shell
    -launcher <launcher> use <launcher> instead of 
                         'gnome-terminal --app-id com.canonical.miral.Terminal'

In addition miral-desktop has the option to set the VT that is used:

    -vt       <termid>   set the virtual terminal [4]

There are some additional options (listed with "-h") but those are the important
ones.

#### MirAL Options

The scripts can also be used to pass options to MirAL: they pass everything on
the command-line following the first thing they don't understand. These can be
listed by `miral-shell --help`. Most of these options are inherited from Mir,
but the following MirAL specific are likely to be of interest:

    --window-management-trace           log trace message

Probably the main use for MirAL is to test window-management (either of a
toolkit or of a server) and this logs all calls to and from the window 
management policy. This option is supported directly in the MirAL library and
works for any MirAL based shell - even one you write yourself.

    --keymap arg (=us)                  keymap <layout>[+<variant>[+<options>]]
                                        , e,g, "gb" or "cz+qwerty" or 
                                        "de++compose:caps"

For those of us not in the USA this is very useful. Both the -shell and -kiosk
examples support this option.

    --window-manager arg (=titlebar)   window management strategy 
                                       [{titlebar|tiling|system-compositor}]

Is only supported by miral-shell and its main use is to allow an alternative
"tiling" window manager to be selected.

Using MirAL for development
---------------------------

Install the headers and libraries for using libmiral in development:

    $ sudo apt install libmiral-dev

A `miral.pc` file is provided for use with `pkg-config` or other tools. For
example: `pkg-config --cflags miral`

The MirAL documentation can be installed and read like this:

    $ sudo apt install miral-doc
    $ xdg-open /usr/share/doc/miral-doc/html/index.html

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
in the previous section. 
