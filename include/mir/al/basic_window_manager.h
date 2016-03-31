/*
 * Copyright © 2015-2016 Canonical Ltd.
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
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIR_ABSTRACTION_BASIC_WINDOW_MANAGER_H_
#define MIR_ABSTRACTION_BASIC_WINDOW_MANAGER_H_

#include "mir/al/window_management_info.h"

#include "mir/geometry/rectangles.h"
#include "mir/shell/abstract_shell.h"
#include "mir/shell/window_manager.h"

#include <map>
#include <mutex>

namespace mir
{

/// This is based on mir/examples, but intended to move to miral after building the necessary abstractions
namespace al
{
using shell::SurfaceSet;

/// The interface through which the policy instructs the controller.
/// These functions assume that the BasicWindowManager data structures can be accessed freely.
/// I.e. should only be invoked by the policy handle_... methods (where any necessary locks are held).
class WindowManagerTools
{
public:
    using SurfaceInfoMap = std::map<std::weak_ptr<scene::Surface>, SurfaceInfo, std::owner_less<std::weak_ptr<scene::Surface>>>;
    using SessionInfoMap = std::map<std::weak_ptr<scene::Session>, SessionInfo, std::owner_less<std::weak_ptr<scene::Session>>>;

    virtual auto build_surface(std::shared_ptr<scene::Session> const& session, scene::SurfaceCreationParameters const& parameters)
    -> SurfaceInfo& = 0;

    virtual auto find_session(std::function<bool(SessionInfo const& info)> const& predicate)
    -> std::shared_ptr<scene::Session> = 0;

    virtual auto info_for(std::weak_ptr<scene::Session> const& session) const -> SessionInfo& = 0;

    //TODO remove this overload
    virtual auto info_for(std::weak_ptr<scene::Surface> const& surface) const -> SurfaceInfo& = 0;

    virtual auto info_for(Surface const& surface) const -> SurfaceInfo& = 0;

    virtual std::shared_ptr<scene::Session> focused_session() const = 0;

    virtual auto focused_surface() const -> Surface = 0;

    virtual void focus_next_session() = 0;

    virtual void set_focus_to(Surface const& surface) = 0;

    virtual auto surface_at(geometry::Point cursor) const -> Surface = 0;

    virtual auto active_display() -> geometry::Rectangle const = 0;

    virtual void forget(Surface const& surface) = 0;

    virtual void raise_tree(std::shared_ptr<scene::Surface> const& root) = 0;

    virtual ~WindowManagerTools() = default;
    WindowManagerTools() = default;
    WindowManagerTools(WindowManagerTools const&) = delete;
    WindowManagerTools& operator=(WindowManagerTools const&) = delete;
};

class WindowManagementPolicy
{
public:
    using SessionInfoMap = typename WindowManagerTools::SessionInfoMap;
    using SurfaceInfoMap = typename WindowManagerTools::SurfaceInfoMap;

    virtual void handle_session_info_updated(SessionInfoMap& session_info, geometry::Rectangles const& displays) = 0;

    virtual void handle_displays_updated(SessionInfoMap& session_info, geometry::Rectangles const& displays) = 0;

    virtual auto handle_place_new_surface(
        std::shared_ptr<scene::Session> const& session,
        scene::SurfaceCreationParameters const& request_parameters)
        -> scene::SurfaceCreationParameters = 0;

    virtual void handle_new_surface(SurfaceInfo& surface_info) = 0;

    virtual void handle_modify_surface(SurfaceInfo& surface_info, shell::SurfaceSpecification const& modifications) = 0;

    virtual void handle_delete_surface(SurfaceInfo& surface_info) = 0;

    virtual auto handle_set_state(SurfaceInfo& surface_info, MirSurfaceState value) -> MirSurfaceState = 0;

    virtual void generate_decorations_for(SurfaceInfo& surface_info) = 0;

    virtual bool handle_keyboard_event(MirKeyboardEvent const* event) = 0;

    virtual bool handle_touch_event(MirTouchEvent const* event) = 0;

    virtual bool handle_pointer_event(MirPointerEvent const* event) = 0;

    virtual void handle_raise_surface(SurfaceInfo& surface_info) = 0;

    virtual ~WindowManagementPolicy() = default;
    WindowManagementPolicy() = default;
    WindowManagementPolicy(WindowManagementPolicy const&) = delete;
    WindowManagementPolicy& operator=(WindowManagementPolicy const&) = delete;
};

/// A policy based window manager.
/// This takes care of the management of any meta implementation held for the sessions and surfaces.
class BasicWindowManager : public virtual shell::WindowManager,
    protected WindowManagerTools
{
protected:
    BasicWindowManager(
        shell::FocusController* focus_controller,
        std::unique_ptr<WindowManagementPolicy> policy);

public:
    using typename WindowManagerTools::SurfaceInfoMap;
    using typename WindowManagerTools::SessionInfoMap;

    auto build_surface(std::shared_ptr<scene::Session> const& session, scene::SurfaceCreationParameters const& parameters)
    -> SurfaceInfo& override;

    void add_session(std::shared_ptr<scene::Session> const& session) override;

    void remove_session(std::shared_ptr<scene::Session> const& session) override;

    auto add_surface(
        std::shared_ptr<scene::Session> const& session,
        scene::SurfaceCreationParameters const& params,
        std::function<frontend::SurfaceId(std::shared_ptr<scene::Session> const& session, scene::SurfaceCreationParameters const& params)> const& build)
    -> frontend::SurfaceId override;

    void modify_surface(
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        shell::SurfaceSpecification const& modifications) override;

    void remove_surface(
        std::shared_ptr<scene::Session> const& session,
        std::weak_ptr<scene::Surface> const& surface) override;

    void forget(Surface const& surface) override;

    void add_display(geometry::Rectangle const& area) override;

    void remove_display(geometry::Rectangle const& area) override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    void handle_raise_surface(
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        uint64_t timestamp) override;

    int set_surface_attribute(
        std::shared_ptr<scene::Session> const& /*session*/,
        std::shared_ptr<scene::Surface> const& surface,
        MirSurfaceAttrib attrib,
        int value) override;

    auto find_session(std::function<bool(SessionInfo const& info)> const& predicate)
    -> std::shared_ptr<scene::Session> override;

    auto info_for(std::weak_ptr<scene::Session> const& session) const -> SessionInfo& override;

    auto info_for(std::weak_ptr<scene::Surface> const& surface) const -> SurfaceInfo& /*override*/;

    auto info_for(Surface const& surface) const -> SurfaceInfo& override;

    std::shared_ptr<scene::Session> focused_session() const override;

    auto focused_surface() const -> Surface override;

    void focus_next_session() override;

    void set_focus_to(Surface const& surface) override;

    auto surface_at(geometry::Point cursor) const -> Surface override;

    auto active_display() -> geometry::Rectangle const override;

    void raise_tree(std::shared_ptr<scene::Surface> const& root) override;

private:
    shell::FocusController* const focus_controller;
    std::unique_ptr<WindowManagementPolicy> const policy;

    std::mutex mutex;
    SessionInfoMap session_info;
    SurfaceInfoMap surface_info;
    geometry::Rectangles displays;
    geometry::Point cursor;
    uint64_t last_input_event_timestamp{0};

    // Cache the builder functor for the convenience of policies - this should become unnecessary
    std::function<Surface(std::shared_ptr<scene::Session> const& session, scene::SurfaceCreationParameters const& params)> surface_builder;

    void update_event_timestamp(MirKeyboardEvent const* kev);
    void update_event_timestamp(MirPointerEvent const* pev);
    void update_event_timestamp(MirTouchEvent const* tev);
};

/// A policy based window manager. This exists to initialize BasicWindowManager and
/// the WMPolicy (in an awkward manner).
/// TODO revisit this initialization sequence.
template<typename WMPolicy>
class WindowManagerBuilder : public BasicWindowManager
{
public:

    template <typename... PolicyArgs>
    WindowManagerBuilder(
        shell::FocusController* focus_controller,
        PolicyArgs&&... policy_args) :
        BasicWindowManager(
            focus_controller,
            build_policy(std::forward<PolicyArgs>(policy_args)...))
    {
    }

private:
    template <typename... PolicyArgs>
    auto build_policy(PolicyArgs&&... policy_args)
    -> std::unique_ptr<WMPolicy>
    {
        return std::unique_ptr<WMPolicy>(
            new WMPolicy(this, std::forward<PolicyArgs>(policy_args)...));
    }
};
}
}

#endif /* MIR_ABSTRACTION_BASIC_WINDOW_MANAGER_H_ */
