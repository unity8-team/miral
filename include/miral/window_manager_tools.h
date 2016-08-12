/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIRAL_WINDOW_MANAGER_TOOLS_H
#define MIRAL_WINDOW_MANAGER_TOOLS_H

#include "miral/application.h"
#include "window_info.h"

#include <mir/geometry/displacement.h>

#include <functional>
#include <memory>

namespace mir
{
namespace scene { class Surface; }
}

namespace miral
{
class Window;
struct WindowInfo;
struct ApplicationInfo;
class WindowSpecification;

class WindowManagerToolsImplementation;

/// Window management functions for querying and updating MirAL's model
class WindowManagerTools
{
public:
    explicit WindowManagerTools(WindowManagerToolsImplementation* tools);
    WindowManagerTools(WindowManagerTools const&);
    WindowManagerTools& operator=(WindowManagerTools const&);
    ~WindowManagerTools();

/** @name Query & Update Model
 *  These functions assume that the BasicWindowManager data structures can be accessed freely.
 *  I.e. they should only be used by a thread that has called the WindowManagementPolicy methods
 *  (where any necessary locks are held) or via a invoke_under_lock() callback.
 *  @{ */

    /** count the applications
     *
     * @return number of applications
     */
    auto count_applications() const -> unsigned int;

    /** execute functor for each application
     *
     * @param functor the functor
     */
    void for_each_application(std::function<void(ApplicationInfo& info)> const& functor);

    /** find an application meeting the predicate
     *
     * @param predicate the predicate
     * @return          the application
     */
    auto find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
    -> Application;

    /** retrieve metadata for an application
     *
     * @param session   the application session
     * @return          the metadata
     */
    auto info_for(std::weak_ptr<mir::scene::Session> const& session) const -> ApplicationInfo&;

    /** retrieve metadata for a window
     *
     * @param surface   the window surface
     * @return          the metadata
     */
    auto info_for(std::weak_ptr<mir::scene::Surface> const& surface) const -> WindowInfo&;

    /** retrieve metadata for a window
     *
     * @param window    the window
     * @return          the metadata
     */
    auto info_for(Window const& window) const -> WindowInfo&;

    /** retrieve metadata for a persistent surface id
     *
     * @param id        the persistent surface id
     * @return          the metadata
     * @throw           invalid_argument or runtime_error if the id is badly formatted/doesn't identify a current window
     */
    auto info_for_window_id(std::string const& id) const -> WindowInfo&;

    /** kill the active application
     *
     * @param sig the signal to send
     */
    void kill_active_application(int sig);

    /// Send close request to the window
    void ask_client_to_close(Window const& window);

    /// retrieve the active window
    auto active_window() const -> Window;

    /** select a new active window based on the hint
     *
     * @param hint  the hint
     * @return      the new active window
     */
    auto select_active_window(Window const& hint) -> Window;

    /// move the active window
    void drag_active_window(mir::geometry::Displacement movement);

    /// make the next application active
    void focus_next_application();

    /// make the next surface active within the active application
    void focus_next_within_application();

    /// Find the topmost window at the cursor
    auto window_at(mir::geometry::Point cursor) const -> Window;

    /// Find the active display area
    auto active_display() -> mir::geometry::Rectangle const;

    /// Raise window and all its children
    void raise_tree(Window const& root);

    /// Apply modifications to a window
    void modify_window(WindowInfo& window_info, WindowSpecification const& modifications);

/** @} */

    /** Multi-thread support
     *  Allows threads that don't hold a lock on the model to acquire one and call the "Update Model"
     *  member functions.
     *  This should NOT be used by a thread that has called the WindowManagementPolicy methods (and
     *  already holds the lock).
     */
    void invoke_under_lock(std::function<void()> const& callback);

private:
    WindowManagerToolsImplementation* tools;
};
}

#endif //MIRAL_WINDOW_MANAGER_TOOLS_H
