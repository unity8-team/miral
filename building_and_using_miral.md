Building and using Miral
========================

These instructions assume that you’re using Ubuntu 16.04LTS or later, I’ve not
earlier Ubuntu versions or other distributions.

You’ll need a few development and utility packages installed, along with the
Mir development packages (if you’re working on a phone or tablet use 
mir-graphics-drivers-android in place of mir-graphics-drivers-desktop):

    $ sudo apt-get install cmake g++ make bzr python-pil
    $ sudo apt-get install mir-graphics-drivers-desktop libmirserver-dev libmirclient-dev
    
With these installed you can checkout and build miral:

    $ bzr branch lp:miral
    $ mkdir miral/build
    $ cd  miral/build
    $ cmake ..
    $ make
    
This creates libmiral.so in the lib directory and an example shell 
(miral-shell) in the bin directory. This can be run directly:

    $ bin/miral-shell
    
With the default options this runs in a window on X (which is convenient for
development). To run independently of X you need to grant access to the 
graphics hardware and specify a VT to run in. For example:

    $ sudo bin/miral-shell --vt 4 --arw-file --file $XDG_RUNTIME_DIR/mir_socket
    
The miral-shell example is simple, don’t expect to see a sophisticated launcher
by default. You can start mir apps from the command-line. For example:
 
    $ GDK_BACKEND=mir gnome-terminal --app-id com.canonical.miral.Terminal
    
That’s right, the GDK toolkit runs on Mir and a lot of standard GTK applications
will “just work”. (Those that assume the existence of an X11 server will have
problems though.)

To exit from miral-shell type Ctrl-Alt-BkSp.