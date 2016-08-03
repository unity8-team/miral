Welcome to MirAL {#mainpage}
================

MirAL is an ABI stable abstraction layer over Mir (and some working examples).

Why MirAL?
----------

Mir is a library for writing Linux display servers and shells that are
independent of the underlying graphics stack. It fits into a similar role as an
X server or Weston (a Wayland server) but was initially motivated by Canonical’s
vision of "convergent" computing.

The Mir project has had some success in meeting Canonical’s immediate needs –
it is running in the Ubuntu Touch phones and tablets, and as an experimental
option for running the Unity8 shell on the desktop.  But because of the
concentration of effort on delivering the features needed for this internal use
it hasn’t really addressed the needs of potential users outside of Canonical.

Mir provides two APIs for users: the "client" API is for applications that run
on Mir and that is largely used by toolkits. There is support for Mir in the GTK
and Qt toolkits, and in SDL. This works pretty well and the Mir client API has
remained backwards compatible for a couple of years and can do so for the
foreseeable future.

The problem is that the server-side ABI compatibility is broken by almost every
release of Mir. This isn’t a big problem for Canonical, as the API is fairly
stable and both Mir and Unity8 are in rapid development: rebuilding Unity8
every time Mir is released is a small overhead. But for independent developers
the lack of a stable ABI is problematic as they cannot easily synchronize their
releases to updates of Mir.

MirAL is the answer to this: It offers an "abstraction layer" written over the
top of the current Mir server API that will provide a stable ABI. There are a
number of other goals that can be addressed at the same time:

  - The API can be considerably narrowed as a lot of things can be customized
    that are of no interest to shell development;
  - A more declarative design style can be followed than the implementation
    focused approach that the Mir server API follows; and,
  - Common facilities can be provided that don’t belong in the Mir libraries.
  
Mir is supported on a range of platforms including phones, tablets and desktops.
There is also work to provide it as a "snap" in the "internet of things". The
miral-shell already works in all these environments (for a suitably forgiving
value of "works") and is an early opportunity to learn about this alternative
to X11.

Notes for Developers
--------------------

 - \ref building_and_using_miral
 - \ref introducing_the_miral_api
 - \ref tasks_for_the_interested_reader