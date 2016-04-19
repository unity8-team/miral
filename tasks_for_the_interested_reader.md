Tasks for the interested reader
===============================

MirAL needs to grow based on feeback on what shell developers really need. The
simple miral-shell example is around the "minimally viable product" point. That
is, it can be used but isn't very satisfying. Also the encapsulation provided
by libmiral is leaky.


Fixing the leaky encapsulation
------------------------------

Currently, some types from the Mir server API are exposed by MirAL. These need
suitable wrapper classes providing, and the corresponding #includes removing
from include/miral/*.h. Finally, the mirserver pkg_check_modules() and  
include_directories() options need removing from miral-server/CMakeLists.txt

 - mir::graphics::DisplayConfigurationOutputId

 - mir::scene::SurfaceCreationParameters and mir::shell::SurfaceSpecification
   these are very similar to each other and overlap with miral::WindowInfo.
   All three could be combined into a single type.
   

ABI design and stability
------------------------

There are some aspects of the current interface that are of concern for the
stability of the ABI. In particular, classes with accessible data members or
with virtual function tables can only be modified in restricted ways.
 
 - miral::WindowInfo and miral::ApplicationInfo these would be better implemented
   using the CheshireCat/pimpl idiom allowing properties to be added without 
   breaking ABI.

 - There's duplicated logic managing the "info" data across the different window
   management policies that should handled by the MirAL BasicWindowManager.
   
 - There are neither tests of MirAL code, nor of the sample code using it.
   (This comes from its origin as a proof-of-concept "spike", but needs
   correcting.) The test doubles supplied by mir-test-tools should be helpful.
   
 - There's a lack of documentation of the preconditions and postconditions of
   the API.
   

Missing functionality
---------------------

To make use of miral-shell more satisfying there are a lot of pieces of work
needed. Some are simple improvements to the sample shell, other may need 
additions to libmiral to expose additional Mir functionality.

 - Titlebars. The default "canonical" window management strategy paints
   grey rectangles for titlebars. The "tiling" window does not offer them
   at all. They should contain the window title and sizing controls.
   
 - Keyboard layouts. It should be possible to configure and use non-US keyboard
   layouts.
   
 - Better integration of startup animation. A short animation is played on
   startup. Ideally this should remain visible until a client is launched,
   fade out over the top of the client and resume when the last client exits.

 - Launching internal clients. Currently a short animation is played on
   startup. Shells ought to be able to launch internal clients at any time.
   
 - launching external clients. There's currently an option to launch the
   gnome-terminal at startup. This would be better with Ctrl-Alt-T. But note, 
   forking from a running server (with multiple threads and owning system
   resources) is a Bad Idea(tm).
   
 - launching startup applications. There's currently an option to launch the
   gnome-terminal at startup. A more general approach is desirable.
   
 - Wallpaper. The default black background is boring.
  
 - Shadows, animations and other "effects".
 
 - Titlebar toolkits. We shouldn't have both the server and client drawing
   these (as can be seen with gnome-terminal).
   
 - Menu integration with toolkits. We should probably support dbus menus.
   
 - Add virtual workspaces. A virtual workspace represents a group of windows
   that can be switched away from (hidden) or to (shown) in a single action.
   Each window belongs in a single virtual workspace. A virtual workspace
   represents a group of windows that can be switched away from (hidden) or 
   to (shown) in a single action. Each window belongs in a single virtual
   workspace.


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
