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

namespace
{
int const title_bar_height = 10;

struct Locker
{
    Locker(std::mutex& mutex, std::unique_ptr<miral::WindowManagementPolicy> const& policy) :
        lock{mutex},
        policy{policy.get()}
    {
        policy->advise_begin();
    }

    ~Locker()
    {
        policy->advise_end();
    }

    std::lock_guard<std::mutex> const lock;
    miral::WindowManagementPolicy* const policy;
};
}

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
    Locker lock{mutex, policy};
    policy->advise_new_app(app_info[session] = ApplicationInfo(session));
}

void miral::BasicWindowManager::remove_session(std::shared_ptr<scene::Session> const& session)
{
    Locker lock{mutex, policy};
    policy->advise_delete_app(app_info[session]);
    app_info.erase(session);
}

auto miral::BasicWindowManager::add_surface(
    std::shared_ptr<scene::Session> const& session,
    scene::SurfaceCreationParameters const& params,
    std::function<frontend::SurfaceId(std::shared_ptr<scene::Session> const& session, scene::SurfaceCreationParameters const& params)> const& build)
-> frontend::SurfaceId
{
    Locker lock{mutex, policy};
    surface_builder = [build](std::shared_ptr<scene::Session> const& session, WindowSpecification const& params)
        {
            scene::SurfaceCreationParameters parameters;
            params.update(parameters);
            return Window{session, build(session, parameters)};
        };

    auto& session_info = info_for(session);

    auto default_placement = place_new_surface(session_info, params);
    auto& window_info = build_window(session, policy->place_new_surface(session_info, default_placement));

    auto const window = window_info.window();

    session_info.add_window(window);

    if (auto const parent = window_info.parent())
        info_for(parent).add_child(window);

    if (window_info.state() == mir_surface_state_fullscreen)
        fullscreen_surfaces.insert(window_info.window());

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
    Locker lock{mutex, policy};
    policy->handle_modify_window(info_for(surface), modifications);
}

void miral::BasicWindowManager::remove_surface(
    std::shared_ptr<scene::Session> const& session,
    std::weak_ptr<scene::Surface> const& surface)
{
    Locker lock{mutex, policy};
    auto& info = info_for(surface);

    policy->advise_delete_window(info);

    bool const is_active_window{mru_active_windows.top() == info.window()};

    auto& session_info = info_for(session);

    session_info.remove_window(info.window());
    mru_active_windows.erase(info.window());
    fullscreen_surfaces.erase(info.window());

    session->destroy_surface(surface);

    // NB erase() invalidates info, but we want to keep access to "parent".
    auto const parent = info.parent();
    erase(info);

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

void miral::BasicWindowManager::erase(miral::WindowInfo const& info)
{
    if (auto const parent = info.parent())
        info_for(parent).remove_child(info.window());

    for (auto& child : info.children())
        info_for(child).parent({});

    window_info.erase(info.window());
}

void miral::BasicWindowManager::add_display(geometry::Rectangle const& area)
{
    Locker lock{mutex, policy};
    displays.add(area);

    for (auto window : fullscreen_surfaces)
    {
        if (window)
        {
            auto& info = info_for(window);
            Rectangle rect{window.top_left(), window.size()};

            graphics::DisplayConfigurationOutputId id{info.output_id()};
            display_layout->place_in_output(id, rect);
            place_and_size(info, rect.top_left, rect.size);
        }
    }

    policy->handle_displays_updated(displays);
}

void miral::BasicWindowManager::remove_display(geometry::Rectangle const& area)
{
    Locker lock{mutex, policy};
    displays.remove(area);
    for (auto window : fullscreen_surfaces)
    {
        if (window)
        {
            auto& info = info_for(window);
            Rectangle rect{window.top_left(), window.size()};

            graphics::DisplayConfigurationOutputId id{info.output_id()};
            display_layout->place_in_output(id, rect);
            place_and_size(info, rect.top_left, rect.size);
        }
    }

    policy->handle_displays_updated(displays);
}

bool miral::BasicWindowManager::handle_keyboard_event(MirKeyboardEvent const* event)
{
    Locker lock{mutex, policy};
    update_event_timestamp(event);
    return policy->handle_keyboard_event(event);
}

bool miral::BasicWindowManager::handle_touch_event(MirTouchEvent const* event)
{
    Locker lock{mutex, policy};
    update_event_timestamp(event);
    return policy->handle_touch_event(event);
}

bool miral::BasicWindowManager::handle_pointer_event(MirPointerEvent const* event)
{
    Locker lock{mutex, policy};
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
    Locker lock{mutex, policy};
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

    Locker lock{mutex, policy};
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

        if (current != end(siblings))
        {
            while (++current != end(siblings) && prev == select_active_window(*current))
                ;
        }

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
    std::vector<Window> windows;

    std::function<void(WindowInfo const& info)> const add_children =
        [&,this](WindowInfo const& info)
            {
                for (auto const& child : info.children())
                {
                    windows.push_back(child);
                    add_children(info_for(child));
                }
            };

    windows.push_back(root);
    add_children(info_for(root));

    policy->advise_raise(windows);
    focus_controller->raise({begin(windows), end(windows)});
}

void miral::BasicWindowManager::move_tree(miral::WindowInfo& root, mir::geometry::Displacement movement)
{
    auto const top_left = root.window().top_left() + movement;

    policy->advise_move_to(root, top_left);
    root.window().move_to(top_left);

    for (auto const& child: root.children())
        move_tree(info_for(child), movement);
}

void miral::BasicWindowManager::modify_window(WindowInfo& window_info, WindowSpecification const& modifications)
{
    auto window_info_tmp = window_info;

#define COPY_IF_SET(field)\
    if (modifications.field().is_set())\
        window_info_tmp.field(modifications.field().value())

    COPY_IF_SET(name);
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
        std::shared_ptr<scene::Surface>(window)->configure(mir_surface_attrib_type, window_info.type());

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
        std::shared_ptr<scene::Surface>(window)->rename(modifications.name().value());

    if (modifications.streams().is_set())
    {
        auto const& config = modifications.streams().value();

        std::vector<shell::StreamSpecification> dest;
        dest.reserve(config.size());

#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 22, 0)
        for (auto const& stream : config)
            dest.push_back(
                shell::StreamSpecification{frontend::BufferStreamId{stream.stream_id.as_value()}, stream.displacement});
#else
        for (auto const& stream : config)
            {
                dest.push_back(
                    mir::shell::StreamSpecification{
                        mir::frontend::BufferStreamId{stream.stream_id.as_value()},
                        stream.displacement,
                        stream.size
                    });
            }
#endif
        window.application()->configure_streams(*std::shared_ptr<scene::Surface>(window), dest);
    }

    if (modifications.input_shape().is_set())
        std::shared_ptr<scene::Surface>(window)->set_input_region(modifications.input_shape().value());

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
        fullscreen_surfaces.erase(window_info.window());
    }
    else
    {
        fullscreen_surfaces.insert(window_info.window());
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
            graphics::DisplayConfigurationOutputId id{window_info.output_id()};
            display_layout->place_in_output(id, rect);
        }
        else
        {
            display_layout->size_to_output(rect);
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

void miral::BasicWindowManager::invoke_under_lock(std::function<void()> const& callback)
{
    Locker lock{mutex, policy};
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

auto miral::BasicWindowManager::place_new_surface(ApplicationInfo const& app_info, WindowSpecification parameters)
-> WindowSpecification
{
    auto surf_type = parameters.type().is_set() ? parameters.type().value() : mir_surface_type_normal;
    bool const needs_titlebar = WindowInfo::needs_titlebar(surf_type);

    if (needs_titlebar)
        parameters.size() = Size{parameters.size().value().width, parameters.size().value().height + DeltaY{title_bar_height}};

    if (!parameters.state().is_set())
        parameters.state() = mir_surface_state_restored;

    auto const active_display_area = active_display();

    auto const width = parameters.size().value().width.as_int();
    auto const height = parameters.size().value().height.as_int();

    bool positioned = false;

    bool const has_parent{parameters.parent().is_set() && parameters.parent().value().lock()};

    if (parameters.output_id().is_set() && parameters.output_id().value() != 0)
    {
        Rectangle rect{parameters.top_left().value(), parameters.size().value()};
        graphics::DisplayConfigurationOutputId id{parameters.output_id().value()};
        display_layout->place_in_output(id, rect);
        parameters.top_left() = rect.top_left;
        parameters.size() = rect.size;
        parameters.state() = mir_surface_state_fullscreen;
        positioned = true;
    }
    else if (!has_parent) // No parent => client can't suggest positioning
    {
        if (app_info.windows().size() > 0)
        {
            if (auto const default_window = app_info.windows()[0])
            {
                static Displacement const offset{title_bar_height, title_bar_height};

                parameters.top_left() = default_window.top_left() + offset;

                Rectangle display_for_app{default_window.top_left(), default_window.size()};

                display_layout->size_to_output(display_for_app);

                positioned = display_for_app.overlaps(Rectangle{parameters.top_left().value(), parameters.size().value()});
            }
        }
    }

    if (has_parent && parameters.aux_rect().is_set() && parameters.edge_attachment().is_set())
    {
        auto parent = info_for(parameters.parent().value()).window();

        auto const edge_attachment = parameters.edge_attachment().value();
        auto const aux_rect = parameters.aux_rect().value();
        auto const parent_top_left = parent.top_left();
        auto const top_left = aux_rect.top_left     -Point{} + parent_top_left;
        auto const top_right= aux_rect.top_right()  -Point{} + parent_top_left;
        auto const bot_left = aux_rect.bottom_left()-Point{} + parent_top_left;

        if (edge_attachment & mir_edge_attachment_vertical)
        {
            if (active_display_area.contains(top_right + Displacement{width, height}))
            {
                parameters.top_left() = top_right;
                positioned = true;
            }
            else if (active_display_area.contains(top_left + Displacement{-width, height}))
            {
                parameters.top_left() = top_left + Displacement{-width, 0};
                positioned = true;
            }
        }

        if (edge_attachment & mir_edge_attachment_horizontal)
        {
            if (active_display_area.contains(bot_left + Displacement{width, height}))
            {
                parameters.top_left() = bot_left;
                positioned = true;
            }
            else if (active_display_area.contains(top_left + Displacement{width, -height}))
            {
                parameters.top_left() = top_left + Displacement{0, -height};
                positioned = true;
            }
        }
    }
    else if (has_parent)
    {
        auto parent = info_for(parameters.parent().value()).window();
        //  o Otherwise, if the dialog is not the same as any previous dialog for the
        //    same parent window, and/or it does not have user-customized position:
        //      o It should be optically centered relative to its parent, unless this
        //        would overlap or cover the title bar of the parent.
        //      o Otherwise, it should be cascaded vertically (but not horizontally)
        //        relative to its parent, unless, this would cause at least part of
        //        it to extend into shell space.
        auto const parent_top_left = parent.top_left();
        auto const centred = parent_top_left
                             + 0.5*(as_displacement(parent.size()) - as_displacement(parameters.size().value()))
                             - DeltaY{(parent.size().height.as_int()-height)/6};

        parameters.top_left() = centred;
        positioned = true;
    }

    if (!positioned)
    {
        auto centred = active_display_area.top_left
                       + 0.5*(as_displacement(active_display_area.size) - as_displacement(parameters.size().value()))
                       - DeltaY{(active_display_area.size.height.as_int()-height)/6};

        switch (parameters.state().value())
        {
        case mir_surface_state_fullscreen:
        case mir_surface_state_maximized:
            parameters.top_left() = active_display_area.top_left;
            parameters.size() = active_display_area.size;
            break;

        case mir_surface_state_vertmaximized:
            centred.y = active_display_area.top_left.y;
            parameters.top_left() = centred;
            parameters.size() = Size{parameters.size().value().width, active_display_area.size.height};
            break;

        case mir_surface_state_horizmaximized:
            centred.x = active_display_area.top_left.x;
            parameters.top_left() = centred;
            parameters.size() = Size{active_display_area.size.width, parameters.size().value().height};
            break;

        default:
            parameters.top_left() = centred;
        }

        auto const display_area = displays.bounding_rectangle();

        if (parameters.top_left().value().y < display_area.top_left.y)
            parameters.top_left() = Point{parameters.top_left().value().x, display_area.top_left.y};
    }

    if (parameters.state().value() != mir_surface_state_fullscreen && needs_titlebar)
    {
        parameters.top_left() = Point{parameters.top_left().value().x, parameters.top_left().value().y + DeltaY{title_bar_height}};
        parameters.size() = Size{parameters.size().value().width, parameters.size().value().height - DeltaY{title_bar_height}};
    }

    return parameters;
}
