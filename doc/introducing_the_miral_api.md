Introducing the Miral API {#introducing_the_miral_api}
=========================

The main() program
------------------

The main() program from miral-shell looks like this:

\include shell_main.cpp

This shell is providing TitlebarWindowManagerPolicy, TilingWindowManagerPolicy
and SpinnerSplash. The rest is from MirAL.

If you look for the corresponding code in lp:qtmir and lp:mir you’ll find it
less clear, more verbose and scattered over multiple files.

A shell has to provide a window management policy (miral-shell provides two: 
TitlebarWindowManagerPolicy and TilingWindowManagerPolicy). A window management
policy needs to implement the WindowManagementPolicy interface for handling a
set of window management events: \ref miral::WindowManagementPolicy

The way these events are handled determines the behaviour of the shell.

The principle interface for controlling Mir is similar: \ref miral::WindowManagerTools
