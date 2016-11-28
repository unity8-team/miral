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

#include "miral/active_outputs.h"
#include "miral/output.h"

#include <mir/version.h>
#include <mir/graphics/display_configuration.h>
#include <mir/server.h>

#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 26, 0)
#include <mir/graphics/display_configuration_report.h>
#else
#include <mir/graphics/display_configuration_observer.h>
#include <mir/observer_registrar.h>
#include <mir/main_loop.h>
#endif

#include <algorithm>
#include <mutex>
#include <vector>

void miral::ActiveOutputsListener::advise_output_begin() {}
void miral::ActiveOutputsListener::advise_output_end() {}
void miral::ActiveOutputsListener::advise_output_create(Output const& /*output*/) {}
void miral::ActiveOutputsListener::advise_output_update(Output const& /*updated*/, Output const& /*original*/) {}
void miral::ActiveOutputsListener::advise_output_delete(Output const& /*output*/) {}

#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 26, 0)
struct miral::ActiveOutputsMonitor::Self : mir::graphics::DisplayConfigurationReport
{
    virtual void initial_configuration(mir::graphics::DisplayConfiguration const& configuration) override;
    virtual void new_configuration(mir::graphics::DisplayConfiguration const& configuration) override;

    std::mutex mutex;
    std::vector<ActiveOutputsListener*> listeners;
    std::vector<Output> outputs;
};
#else
struct miral::ActiveOutputsMonitor::Self : mir::graphics::DisplayConfigurationObserver
{
    void initial_configuration(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& configuration) override;
    void configuration_applied(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& config) override;

    void configuration_failed(
        std::shared_ptr<mir::graphics::DisplayConfiguration const> const&,
        std::exception const&) override
    {
    }

    void catastrophic_configuration_error(
        std::shared_ptr<mir::graphics::DisplayConfiguration const> const&,
        std::exception const&) override
    {
    }

    void base_configuration_updated(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& ) override {}

    std::mutex mutex;
    std::vector<ActiveOutputsListener*> listeners;
    std::vector<Output> outputs;
};
#endif

miral::ActiveOutputsMonitor::ActiveOutputsMonitor() :
    self{std::make_shared<Self>()}
{
}

miral::ActiveOutputsMonitor::~ActiveOutputsMonitor() = default;
miral::ActiveOutputsMonitor::ActiveOutputsMonitor(ActiveOutputsMonitor const&) = default;
miral::ActiveOutputsMonitor& miral::ActiveOutputsMonitor::operator=(ActiveOutputsMonitor const&) = default;

void miral::ActiveOutputsMonitor::add_listener(ActiveOutputsListener* listener)
{
    std::lock_guard<decltype(self->mutex)> lock{self->mutex};

    self->listeners.push_back(listener);
}

void miral::ActiveOutputsMonitor::delete_listener(ActiveOutputsListener* listener)
{
    std::lock_guard<decltype(self->mutex)> lock{self->mutex};

    auto const new_end = std::remove(self->listeners.begin(), self->listeners.end(), listener);
    self->listeners.erase(new_end, self->listeners.end());
}

void miral::ActiveOutputsMonitor::operator()(mir::Server& server)
{
    std::lock_guard<decltype(self->mutex)> lock{self->mutex};

#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 26, 0)
    server.override_the_display_configuration_report([this]{ return self; });
#else
    server.add_pre_init_callback([this, &server]
        { server.the_display_configuration_observer_registrar()->register_interest(self); });
#endif
}

void miral::ActiveOutputsMonitor::for_each_output(
    std::function<void(std::vector<Output> const& outputs)> const& functor) const
{
    std::lock_guard<decltype(self->mutex)> lock{self->mutex};
    functor(self->outputs);
}

#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 26, 0)
void miral::ActiveOutputsMonitor::Self::initial_configuration(mir::graphics::DisplayConfiguration const& configuration)
{
    new_configuration(configuration);
}
#else
void miral::ActiveOutputsMonitor::Self::initial_configuration(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& configuration)
{
    configuration_applied(configuration);
}
#endif

#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 26, 0)
void miral::ActiveOutputsMonitor::Self::new_configuration(mir::graphics::DisplayConfiguration const& configuration)
{
    std::lock_guard<decltype(mutex)> lock{mutex};

    decltype(outputs) current_outputs;

    for (auto const l : listeners)
        l->advise_output_begin();

    configuration.for_each_output([&current_outputs, this](mir::graphics::DisplayConfigurationOutput const& output)
        {
            Output o{output};

            if (!o.connected() || !o.valid()) return;

            auto op = find_if(begin(outputs), end(outputs), [&](Output const& oo) { return oo.is_same_output(o); });

            if (op == end(outputs))
            {
                for (auto const l : listeners)
                    l->advise_output_create(o);
            }
            else if (!equivalent_display_area(o, *op))
            {
                for (auto const l : listeners)
                    l->advise_output_update(o, *op);
            }

            current_outputs.push_back(o);
        });

    for (auto const& o : outputs)
    {
        auto op = find_if(begin(current_outputs), end(current_outputs), [&](Output const& oo)
            { return oo.is_same_output(o); });

        if (op == end(current_outputs))
            for (auto const l : listeners)
                l->advise_output_delete(o);
    }

    current_outputs.swap(outputs);
    for (auto const l : listeners)
        l->advise_output_end();
}
#else
void miral::ActiveOutputsMonitor::Self::configuration_applied(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& config)
{
    std::lock_guard<decltype(mutex)> lock{mutex};

    decltype(outputs) current_outputs;

    for (auto const l : listeners)
        l->advise_output_begin();

    config->for_each_output(
        [&current_outputs, this](mir::graphics::DisplayConfigurationOutput const& output)
            {
            Output o{output};

            if (!o.connected() || !o.valid()) return;

            auto op = find_if(
                begin(outputs), end(outputs), [&](Output const& oo)
                    { return oo.is_same_output(o); });

            if (op == end(outputs))
            {
                for (auto const l : listeners)
                    l->advise_output_create(o);
            }
            else if (!equivalent_display_area(o, *op))
            {
                for (auto const l : listeners)
                    l->advise_output_update(o, *op);
            }

            current_outputs.push_back(o);
            });

    for (auto const& o : outputs)
    {
        auto op = find_if(
            begin(current_outputs), end(current_outputs), [&](Output const& oo)
                { return oo.is_same_output(o); });

        if (op == end(current_outputs))
            for (auto const l : listeners)
                l->advise_output_delete(o);
    }

    current_outputs.swap(outputs);
    for (auto const l : listeners)
        l->advise_output_end();
}
#endif
