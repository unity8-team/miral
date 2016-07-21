Tasks for the interested reader
===============================

MirAL needs to grow based on feeback on what shell developers really need. The
simple miral-shell example is around the "minimally viable product" point. That
is, it can be used but isn't very satisfying. Also the encapsulation provided
by libmiral is leaky.

ABI design and stability
------------------------

There are some aspects of the current interface that are of concern for the
stability of the ABI. In particular, classes with accessible data members or
with virtual function tables can only be modified in restricted ways.
 
 - There are neither tests of MirAL code, nor of the sample code using it.
   (This comes from its origin as a proof-of-concept "spike", but needs
   correcting.) The test doubles supplied by mir-test-tools should be helpful.
   (NB the version of mir-test-tools in Xenial is broken. lp:1583536)
   
 - There's a lack of documentation of the preconditions and postconditions of
   the API.
   
Missing functionality
---------------------

To make use of miral-shell more satisfying there are a lot of pieces of work
needed. Some are simple improvements to the sample shell, other may need 
additions to libmiral to expose additional Mir functionality.

 - Titlebars. The default "titlebar" window management strategy paints
   grey rectangles for titlebars.  They should contain the window title and
   sizing controls.
   
 - Titlebars. The "tiling" window does not offer them at all.
   
 - Titlebars. GTK+ apps provide their own titlebars, better integration is
   needed.
   
 - Keyboard layouts. It should be possible to configure and use non-US keyboard
   layouts.
   
 - Better integration of startup animation. A short animation is played on
   startup. Ideally this should remain visible until a client is launched,
   fade out over the top of the client and resume when the last client exits.

 - launching external clients. There's currently an option to launch e.g. the
   gnome-terminal at startup. This would be better with Ctrl-Alt-T. But note, 
   forking from a running server (with multiple threads and owning system
   resources) is a Bad Idea(tm).
   
 - Wallpaper. The default black background is boring.
  
 - Shadows, animations and other "effects".
 
 - Menu integration with toolkits. We should probably support dbus menus.
   
 - Add virtual workspaces. A virtual workspace represents a group of windows
   that can be switched away from (hidden) or to (shown) in a single action.
   Each window belongs in a single virtual workspace. A virtual workspace
   represents a group of windows that can be switched away from (hidden) or 
   to (shown) in a single action. Each window belongs in a single virtual
   workspace.
   
 - Compositing effects. There needs to be an API for coding "effects" within
   the compositor (spreads, wobbly windows, shadows).

 - Customizing compositing. There needs to be a mechanism for loading custom
   compositing effects. E.g. specifying a module (or modules) to load.
   
 - Display configuration. There needs to be a useful abstraction of outputs
   and geometry. See miral-qt/src/modules/Unity/Screens/screens.h and 
   ScreensModel::update() in miral-qt/src/platforms/mirserver/screensmodel.cpp.
   (Or maybe an internal client can use the client APIs?)
   
 - Cursor images. lp:qtmir stubs the cursor images and paints the cursor in its
   compositor. Need to consider what ought to be supported here.

The tiling window management strategy
-------------------------------------

This code was originally written to prove that different strategies are 
possible, without much consideration of being useful. Here are some suggestions
for a better approach:

 - top level windows ought to fill the tile when created
 
 - the tiling algorithm ought to lay windows out as follows:
 
    - Single window: takes up the whole screen
    
    - Two windows: The screen is split in two tiles of equal width (half the
      screen’s width) and full height. Each window is placed in a tile (left
      or right), with the newest window occupying the left tile.
      
    - Three or more windows: The screen is split in two tiles of equal width
      and full height, as in the previous case. The newest window occupies
      the left tile. The right part is now further divided vertically into
      smaller tiles having equal height, to host the remaining windows, with
      older windows being closer to the bottom.

  - Add a titlebar to the top of the screen. The titlebar should be split evenly
    into horizontal blocks, one per tile. Each block containing the title of the
    top-level window. The focussed tile is highlighted. Clicking on a title
    selects the corresponding window.
    
Art Resource
------------

Anyone who is able to create and propose some examples to then pick from. We
currently need:

 - Default wallpaper

 - Default icon (using ubuntu logo atm). Must be svg
 
Developer Tools
---------------

When writing tests it would be useful to be able insert wrappers around the
WindowManagerTools and WindowManagementPolicy to trace the calls.

When manually testing it would be useful to be able to have logging wrappers 
around the WindowManagerTools and WindowManagementPolicy to trace the calls.
 
  - A wrapper class for WindowManagerTools
  - A wrapper class for WindowManagementPolicy
  - A logging extension to the WindowManagerTools wrapper
  - A logging extension to the WindowManagementPolicy wrapper
  - An option to insert the above wrappers at runtime
  - A feature to insert mock "expectations" into WindowInfo
  - A feature to insert mock "expectations" into Window
  - An option to insert logging into WindowInfo
  - An option to insert logging into Window
