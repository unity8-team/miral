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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "basic_window_manager.h"

#include <mir/scene/session.h>
#include <mir/scene/surface.h>
#include <mir/scene/surface_creation_parameters.h>
#include <mir/shell/display_layout.h>
#include <mir/shell/surface_ready_observer.h>
#include <mir/version.h>

#include <algorithm>

using namespace mir;

miral::BasicWindowManager::BasicWindowManager(
    shell::FocusController* focus_controller,
    std::shared_ptr<shell::DisplayLayout> const& display_layout,
    WindowManagementPolicyBuilder const& build) :
    focus_controller(focus_controller),
    display_layout(display_layout),
    policy(build(this)),
    surface_builder([](std::shared_ptr<scene::Session> const&, WindowSpecification const&) -> Window
        { throw std::logic_error{"Can't create a window yet"};})
{
}

auto miral::BasicWindowManager::build_window(Application const& application, WindowSpecification const& spec_)
-> WindowInfo&
{
    auto spec = spec_;

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 22, 0)
    // Quick, dirty hack to support titlebar creation - really need an API for buffer stream creation
    if (!spec.content_id().is_set() && !spec.streams().is_set())
    {
        mir::graphics::BufferProperties properties(spec.size().value(), spec.pixel_format().value(), mir::graphics::BufferUsage::software);
        spec.content_id() = BufferStreamId{application->create_buffer_stream(properties).as_value()};
    }
#endif

    auto result = surface_builder(application, spec);
    auto& info = window_info.emplace(result, WindowInfo{result, spec}).first->second;
    if (spec.parent().is_set() && spec.parent().value().lock())
        info.parent(info_for(spec.parent().value()).window());
    return info;
}

void miral::BasicWindowManager::add_session(std::shared_ptr<scene::Session> const& session)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    app_info[session] = ApplicationInfo();
    policy->handle_app_info_updated(displays);
}

void miral::BasicWindowManager::remove_session(std::shared_ptr<scene::Session> const& session)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    app_info.erase(session);
    policy->handle_app_info_updated(displays);
}

auto miral::BasicWindowManager::add_surface(
    std::shared_ptr<scene::Session> const& session,
    scene::SurfaceCreationParameters const& params,
    std::function<frontend::SurfaceId(std::shared_ptr<scene::Session> const& session, scene::SurfaceCreationParameters const& params)> const& build)
-> frontend::SurfaceId
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    surface_builder = [build](std::shared_ptr<scene::Session> const& session, WindowSpecification const& params)
        {
            scene::SurfaceCreationParameters parameters;
            params.update(parameters);
            return Window{session, build(session, parameters)};
        };

    auto& session_info = info_for(session);
    auto& window_info = build_window(session, policy->place_new_surface(session_info, params));

    auto const window = window_info.window();

    session_info.add_window(window);

    if (auto const parent = window_info.parent())
        info_for(parent).add_child(window);

    policy->advise_new_window(window_info);

    if (window_info.can_be_active())
    {
        std::shared_ptr<scene::Surface> const scene_surface = window_info.window();
        scene_surface->add_observer(std::make_shared<shell::SurfaceReadyObserver>(
            [this, &window_info](std::shared_ptr<scene::Session> const&, std::shared_ptr<scene::Surface> const&)
                { policy->handle_window_ready(window_info); },
            session,
            scene_surface));
    }

    return window_info.window().surface_id();
}

void miral::BasicWindowManager::modify_surface(
    std::shared_ptr<scene::Session> const& /*application*/,
    std::shared_ptr<scene::Surface> const& surface,
    shell::SurfaceSpecification const& modifications)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    policy->handle_modify_window(info_for(surface), modifications);
}

void miral::BasicWindowManager::remove_surface(
    std::shared_ptr<scene::Session> const& session,
    std::weak_ptr<scene::Surface> const& surface)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    auto& info = info_for(surface);

    bool const is_active_window{mru_active_windows.top() == info.window()};

    if (auto const parent = info.parent())
        info_for(parent).remove_child(info.window());

    auto& session_info = info_for(session);

    session_info.remove_window(info.window());
    mru_active_windows.erase(info.window());

    policy->advise_delete_window(info);

    session->destroy_surface(surface);

    auto const parent = info.parent();

    // NB this invalidates info, but we want to keep access to "parent".
    window_info.erase(surface);

    if (is_active_window)
    {
        // Try to make the parent active
        if (parent && select_active_window(parent))
            return;

        if (can_activate_window_for_session(session))
            return;

        // Try to activate to recently active window of any application
        {
            Window new_focus;

            mru_active_windows.enumerate([&](Window& window)
            {
                // select_active_window() calls set_focus_to() which updates mru_active_windows and changes window
                auto const w = window;
                return !(new_focus = select_active_window(w));
            });

            if (new_focus) return;
        }

        // Fallback to cycling through applications
        focus_next_application();
    }
}

void miral::BasicWindowManager::destroy(Window& window)
{
    window.application()->destroy_surface(window.surface_id());
    window_info.erase(window);
}

void miral::BasicWindowManager::add_display(geometry::Rectangle const& area)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    displays.add(area);
    policy->handle_displays_updated(displays);
}

void miral::BasicWindowManager::remove_display(geometry::Rectangle const& area)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    displays.remove(area);
    policy->handle_displays_updated(displays);
}

bool miral::BasicWindowManager::handle_keyboard_event(MirKeyboardEvent const* event)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    update_event_timestamp(event);
    return policy->handle_keyboard_event(event);
}

bool miral::BasicWindowManager::handle_touch_event(MirTouchEvent const* event)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    update_event_timestamp(event);
    return policy->handle_touch_event(event);
}

bool miral::BasicWindowManager::handle_pointer_event(MirPointerEvent const* event)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    update_event_timestamp(event);

    cursor = {
        mir_pointer_event_axis_value(event, mir_pointer_axis_x),
        mir_pointer_event_axis_value(event, mir_pointer_axis_y)};

    return policy->handle_pointer_event(event);
}

void miral::BasicWindowManager::handle_raise_surface(
    std::shared_ptr<scene::Session> const& /*application*/,
    std::shared_ptr<scene::Surface> const& surface,
    uint64_t timestamp)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    if (timestamp >= last_input_event_timestamp)
        policy->handle_raise_window(info_for(surface));
}

int miral::BasicWindowManager::set_surface_attribute(
    std::shared_ptr<scene::Session> const& /*application*/,
    std::shared_ptr<scene::Surface> const& surface,
    MirSurfaceAttrib attrib,
    int value)
{
    WindowSpecification modification;
    switch (attrib)
    {
    case mir_surface_attrib_type:
        modification.type() = MirSurfaceType(value);
        break;
    case mir_surface_attrib_state:
        modification.state() = MirSurfaceState(value);
        break;

    case mir_surface_attrib_preferred_orientation:
        modification.preferred_orientation() = MirOrientationMode(value);
        break;

    case mir_surface_attrib_visibility:
        // The client really shouldn't be trying to set this.
        // But, as the legacy API exists, we treat it as a query
        return surface->query(mir_surface_attrib_visibility);

    case mir_surface_attrib_focus:
        // The client really shouldn't be trying to set this.
        // But, as the legacy API exists, we treat it as a query
        return surface->query(mir_surface_attrib_focus);

    case mir_surface_attrib_swapinterval:
    case mir_surface_attrib_dpi:
    default:
        return surface->configure(attrib, value);
    }

    std::lock_guard<decltype(mutex)> lock(mutex);
    auto& info = info_for(surface);
    policy->handle_modify_window(info, modification);

    switch (attrib)
    {
    case mir_surface_attrib_type:
        return info.type();

    case mir_surface_attrib_state:
        return info.state();

    case mir_surface_attrib_preferred_orientation:
        return info.preferred_orientation();
        break;

    default:
        return 0; // Can't get here anyway
    }
}

auto miral::BasicWindowManager::count_applications() const
-> unsigned int
{
    return app_info.size();
}


void miral::BasicWindowManager::for_each_application(std::function<void(ApplicationInfo& info)> const& functor)
{
    for(auto& info : app_info)
    {
        functor(info.second);
    }
}

auto miral::BasicWindowManager::find_application(std::function<bool(ApplicationInfo const& info)> const& predicate)
-> Application
{
    for(auto& info : app_info)
    {
        if (predicate(info.second))
        {
            return Application{info.first};
        }
    }

    return Application{};
}

auto miral::BasicWindowManager::info_for(std::weak_ptr<scene::Session> const& session) const
-> ApplicationInfo&
{
    return const_cast<ApplicationInfo&>(app_info.at(session));
}

auto miral::BasicWindowManager::info_for(std::weak_ptr<scene::Surface> const& surface) const
-> WindowInfo&
{
    return const_cast<WindowInfo&>(window_info.at(surface));
}

auto miral::BasicWindowManager::info_for(Window const& window) const
-> WindowInfo&
{
    return info_for(std::weak_ptr<mir::scene::Surface>(window));
}

void miral::BasicWindowManager::kill_active_application(int sig)
{
    if (auto const application = focus_controller->focused_session())
        miral::kill(application, sig);
}

auto miral::BasicWindowManager::active_window() const -> Window
{
    return mru_active_windows.top();
}

void miral::BasicWindowManager::focus_next_application()
{
    focus_controller->focus_next_session();

    if (can_activate_window_for_session(focus_controller->focused_session()))
        return;

    // Last resort: accept wherever focus_controller placed focus
    auto const focussed_surface = focus_controller->focused_surface();
    select_active_window(focussed_surface ? info_for(focussed_surface).window() : Window{});
}

void miral::BasicWindowManager::focus_next_within_application()
{
    if (auto const prev = active_window())
    {
        auto const& siblings = info_for(prev.application()).windows();
        auto current = find(begin(siblings), end(siblings), prev);

        while (current != end(siblings) && prev == select_active_window(*current))
            ++current;

        if (current == end(siblings))
        {
            current = begin(siblings);
            while (prev != *current && prev == select_active_window(*current))
                ++current;
        }
    }
}

auto miral::BasicWindowManager::window_at(geometry::Point cursor) const
-> Window
{
    auto surface_at = focus_controller->surface_at(cursor);
    return surface_at ? info_for(surface_at).window() : Window{};
}

auto miral::BasicWindowManager::active_display()
-> geometry::Rectangle const
{
    geometry::Rectangle result;

    // 1. If a window has input focus, whichever display contains the largest
    //    proportion of the area of that window.
    if (auto const surface = focus_controller->focused_surface())
    {
        auto const surface_rect = surface->input_bounds();
        int max_overlap_area = -1;

        for (auto const& display : displays)
        {
            auto const intersection = surface_rect.intersection_with(display).size;
            if (intersection.width.as_int()*intersection.height.as_int() > max_overlap_area)
            {
                max_overlap_area = intersection.width.as_int()*intersection.height.as_int();
                result = display;
            }
        }
        return result;
    }

    // 2. Otherwise, if any window previously had input focus, for the window that had
    //    it most recently, the display that contained the largest proportion of the
    //    area of that window at the moment it closed, as long as that display is still
    //    available.

    // 3. Otherwise, the display that contains the pointer, if there is one.
    for (auto const& display : displays)
    {
        if (display.contains(cursor))
        {
            // Ignore the (unspecified) possiblity of overlapping displays
            return display;
        }
    }

    // 4. Otherwise, the primary display, if there is one (for example, the laptop display).

    // 5. Otherwise, the first display.
    if (displays.size())
        result = *displays.begin();

    return result;
}

void miral::BasicWindowManager::raise_tree(Window const& root)
{
    SurfaceSet windows;
    std::function<void(std::weak_ptr<scene::Surface> const& surface)> const add_children =
        [&,this](std::weak_ptr<scene::Surface> const& surface)
            {
            auto const& info = info_for(surface);
            windows.insert(begin(info.children()), end(info.children()));
            for (auto const& child : info.children())
                add_children(child);
            };

    windows.insert(root);
    add_children(root);

    focus_controller->raise(windows);
}

void miral::BasicWindowManager::move_tree(miral::WindowInfo& root, mir::geometry::Displacement movement)
{
    root.window().move_to(root.window().top_left() + movement);

    for (auto const& child: root.children())
        move_tree(info_for(child), movement);
}

void miral::BasicWindowManager::modify_window(WindowInfo& window_info, WindowSpecification const& modifications)
{
    auto window_info_tmp = window_info;

#define COPY_IF_SET(field)\
    if (modifications.field().is_set())\
        window_info_tmp.field(modifications.field().value())

    COPY_IF_SET(type);
    COPY_IF_SET(min_width);
    COPY_IF_SET(min_height);
    COPY_IF_SET(max_width);
    COPY_IF_SET(max_height);
    COPY_IF_SET(width_inc);
    COPY_IF_SET(height_inc);
    COPY_IF_SET(min_aspect);
    COPY_IF_SET(max_aspect);
    COPY_IF_SET(output_id);
    COPY_IF_SET(preferred_orientation);

#undef COPY_IF_SET

    if (modifications.parent().is_set())
        window_info_tmp.parent(info_for(modifications.parent().value()).window());

    if (window_info.type() != window_info_tmp.type())
    {
        auto const new_type = window_info_tmp.type();

        if (!window_info.can_morph_to(new_type))
        {
            throw std::runtime_error("Unsupported window type change");
        }

        if (window_info_tmp.must_not_have_parent())
        {
            if (modifications.parent().is_set())
                throw std::runtime_error("Target window type does not support parent");

            window_info_tmp.parent({});
        }
        else if (window_info_tmp.must_have_parent())
        {
            if (!window_info_tmp.parent())
                throw std::runtime_error("Target window type requires parent");
        }
    }

    std::swap(window_info_tmp, window_info);

    auto& window = window_info.window();

    if (window_info.type() != window_info_tmp.type())
        window.set_type(window_info.type());

    if (window_info.parent() != window_info_tmp.parent())
    {
        if (window_info_tmp.parent())
        {
            auto& parent_info = info_for(window_info_tmp.parent());
            parent_info.remove_child(window);
        }

        if (window_info.parent())
        {
            auto& parent_info = info_for(window_info.parent());
            parent_info.add_child(window);
        }
    }

    if (modifications.name().is_set())
        window.rename(modifications.name().value());

    if (modifications.streams().is_set())
        window.configure_streams(modifications.streams().value());

    if (modifications.input_shape().is_set())
        window.set_input_region(modifications.input_shape().value());

    if (modifications.size().is_set())
    {
        Point new_pos = window.top_left();
        Size new_size = modifications.size().value();

        window_info.constrain_resize(new_pos, new_size);
        place_and_size(window_info, new_pos, new_size);
    }
    else if (modifications.min_width().is_set() || modifications.min_height().is_set() ||
             modifications.max_width().is_set() || modifications.max_height().is_set() ||
             modifications.width_inc().is_set() || modifications.height_inc().is_set())
    {
        Point new_pos = window.top_left();
        Size new_size = window.size();

        window_info.constrain_resize(new_pos, new_size);
        place_and_size(window_info, new_pos, new_size);
    }

    if (modifications.state().is_set())
    {
        set_state(window_info, modifications.state().value());
    }
}

void miral::BasicWindowManager::place_and_size(WindowInfo& root, Point const& new_pos, Size const& new_size)
{
    policy->advise_resize(root, new_size);
    root.window().resize(new_size);
    move_tree(root, new_pos - root.window().top_left());
}

void miral::BasicWindowManager::set_state(miral::WindowInfo& window_info, MirSurfaceState value)
{
    switch (value)
    {
    case mir_surface_state_restored:
    case mir_surface_state_maximized:
    case mir_surface_state_vertmaximized:
    case mir_surface_state_horizmaximized:
    case mir_surface_state_fullscreen:
    case mir_surface_state_hidden:
    case mir_surface_state_minimized:
        break;

    default:
        window_info.window().set_state(window_info.state());
        return;
    }

    if (window_info.state() == mir_surface_state_restored)
    {
        window_info.restore_rect({window_info.window().top_left(), window_info.window().size()});
    }

    if (window_info.state() != mir_surface_state_fullscreen)
    {
        window_info.output_id({});
    }

    if (window_info.state() == value)
    {
        return;
    }

    auto const old_pos = window_info.window().top_left();
    Displacement movement;

    policy->advise_state_change(window_info, value);

    auto const display_area = displays.bounding_rectangle();

    switch (value)
    {
    case mir_surface_state_restored:
        movement = window_info.restore_rect().top_left - old_pos;
        window_info.window().resize(window_info.restore_rect().size);
        break;

    case mir_surface_state_maximized:
        movement = display_area.top_left - old_pos;
        window_info.window().resize(display_area.size);
        break;

    case mir_surface_state_horizmaximized:
        movement = Point{display_area.top_left.x, window_info.restore_rect().top_left.y} - old_pos;
        window_info.window().resize({display_area.size.width, window_info.restore_rect().size.height});
        break;

    case mir_surface_state_vertmaximized:
        movement = Point{window_info.restore_rect().top_left.x, display_area.top_left.y} - old_pos;
        window_info.window().resize({window_info.restore_rect().size.width, display_area.size.height});
        break;

    case mir_surface_state_fullscreen:
    {
        Rectangle rect{old_pos, window_info.window().size()};

        if (window_info.has_output_id())
        {
            place_in_output(window_info.output_id(), rect);
        }
        else
        {
            size_to_output(rect);
        }

        movement = rect.top_left - old_pos;
        window_info.window().resize(rect.size);
        break;
    }

    case mir_surface_state_hidden:
    case mir_surface_state_minimized:
        window_info.window().hide();
        window_info.state(value);
        window_info.window().set_state(window_info.state());
        return;

    default:
        break;
    }

    move_tree(window_info, movement);

    window_info.state(value);

    if (window_info.is_visible())
        window_info.window().show();

    window_info.window().set_state(window_info.state());
}


void miral::BasicWindowManager::update_event_timestamp(MirKeyboardEvent const* kev)
{
    auto iev = mir_keyboard_event_input_event(kev);
    last_input_event_timestamp = mir_input_event_get_event_time(iev);
}

void miral::BasicWindowManager::update_event_timestamp(MirPointerEvent const* pev)
{
    auto iev = mir_pointer_event_input_event(pev);
    auto pointer_action = mir_pointer_event_action(pev);

    if (pointer_action == mir_pointer_action_button_up ||
        pointer_action == mir_pointer_action_button_down)
    {
        last_input_event_timestamp = mir_input_event_get_event_time(iev);
    }
}

void miral::BasicWindowManager::update_event_timestamp(MirTouchEvent const* tev)
{
    auto iev = mir_touch_event_input_event(tev);
    auto touch_count = mir_touch_event_point_count(tev);
    for (unsigned i = 0; i < touch_count; i++)
    {
        auto touch_action = mir_touch_event_action(tev, i);
        if (touch_action == mir_touch_action_up ||
            touch_action == mir_touch_action_down)
        {
            last_input_event_timestamp = mir_input_event_get_event_time(iev);
            break;
        }
    }
}

void miral::BasicWindowManager::size_to_output(mir::geometry::Rectangle& rect)
{
    display_layout->size_to_output(rect);
}

bool miral::BasicWindowManager::place_in_output(int id, mir::geometry::Rectangle& rect)
{
    return display_layout->place_in_output(mir::graphics::DisplayConfigurationOutputId{id}, rect);
}

void miral::BasicWindowManager::invoke_under_lock(std::function<void()> const& callback)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    callback();
}

auto miral::BasicWindowManager::select_active_window(Window const& hint) -> miral::Window
{
    auto const prev_window = active_window();

    if (!hint)
    {
        if (prev_window)
        {
            focus_controller->set_focus_to(hint.application(), hint);
            policy->advise_focus_lost(info_for(prev_window));
        }

        return hint;
    }

    auto const& info_for_hint = info_for(hint);

    if (info_for_hint.can_be_active())
    {
        mru_active_windows.push(hint);
        focus_controller->set_focus_to(hint.application(), hint);

        if (prev_window && prev_window != hint)
            policy->advise_focus_lost(info_for(prev_window));

        policy->advise_focus_gained(info_for_hint);
        return hint;
    }
    else
    {
        // Cannot have input focus - try the parent
        if (auto const parent = info_for_hint.parent())
            return select_active_window(parent);
    }

    return {};
}

void miral::BasicWindowManager::drag_active_window(mir::geometry::Displacement movement)
{
    auto const window = active_window();

    if (!window)
        return;

    auto& window_info = info_for(window);

    // placeholder - constrain onscreen

    switch (window_info.state())
    {
    case mir_surface_state_restored:
        break;

        // "A vertically maximised window is anchored to the top and bottom of
        // the available workspace and can have any width."
    case mir_surface_state_vertmaximized:
        movement.dy = DeltaY(0);
        break;

        // "A horizontally maximised window is anchored to the left and right of
        // the available workspace and can have any height"
    case mir_surface_state_horizmaximized:
        movement.dx = DeltaX(0);
        break;

        // "A maximised window is anchored to the top, bottom, left and right of the
        // available workspace. For example, if the launcher is always-visible then
        // the left-edge of the window is anchored to the right-edge of the launcher."
    case mir_surface_state_maximized:
    case mir_surface_state_fullscreen:
    default:
        return;
    }

    move_tree(window_info, movement);
}

auto miral::BasicWindowManager::can_activate_window_for_session(miral::Application const& session) -> bool
{
    miral::Window new_focus;

    mru_active_windows.enumerate([&](miral::Window& window)
        {
            // select_active_window() calls set_focus_to() which updates mru_active_windows and changes window
            auto const w = window;
            return w.application() != session || !(new_focus = miral::BasicWindowManager::select_active_window(w));
        });

    return new_focus;
}