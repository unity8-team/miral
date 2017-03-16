Tasks for the interested reader  {#tasks_for_the_interested_reader}
===============================

MirAL needs to grow based on feeback on what shell developers really need. The
simple miral-shell example is around the "minimally viable product" point. That
is, it can be used but isn't very satisfying. Also the encapsulation provided
by libmiral is leaky.

Missing functionality
---------------------

To make use of miral-shell more satisfying there are a lot of pieces of work
needed. Some are simple improvements to the sample shell, other may need 
additions to libmiral to expose additional Mir functionality.

 - Titlebars. The default "titlebar" window management strategy paints
   grey rectangles for titlebars.  They should contain sizing controls.
   
 - Titlebars. The "tiling" window does not offer them at all.
   
 - Titlebars. GTK+ apps provide their own titlebars, better integration is
   needed.
   
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
   
 - Enabling (disabling) cursor when pointing devices are (not) present.
    - ability to track connected input devices
    - ability to control cursor visibility
   
 - Cut&Paste/Drag&Drop toolkits expect this functionality, but it isn't
   provided by Mir. We ought to find a way to provide this.

The tiling window management strategy
-------------------------------------

This code was originally written to prove that different strategies are 
possible, without much consideration of being useful. Here are some suggestions
for a better approach:

 - Add a titlebar to the top of the screen. The titlebar should be split evenly
   into horizontal blocks, one per tile. Each block containing the title of the
   top-level window. The focused tile is highlighted. Clicking on a title
   selects the corresponding window.

Art Resource
------------

Anyone who is able to create and propose some examples to then pick from. We
currently need:

 - Default wallpaper

 - Default icon (using ubuntu logo atm). Must be svg
