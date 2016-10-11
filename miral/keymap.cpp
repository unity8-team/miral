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

#include "miral/keymap.h"

#include <mir/input/input_device_observer.h>
#include <mir/input/input_device_hub.h>
#include <mir/input/device.h>
#include <mir/server.h>
#include <mir/version.h>

#if MIR_VERSION >= MIR_VERSION_NUMBER(0, 24, 1)
#include <mir/input/keymap.h>
#endif

#include <algorithm>
#include <vector>

struct miral::Keymap::Self : mir::input::InputDeviceObserver 
{
    Self(std::string const& keymap) : keymap{keymap}{}

    void device_added(std::shared_ptr<mir::input::Device> const& device) override
    {
        if (mir::contains(device->capabilities(), mir::input::DeviceCapability::keyboard))
            add_keyboard(device);
    }

    void device_changed(std::shared_ptr<mir::input::Device> const& device) override
    {
        auto const keyboard = std::find(begin(keyboards), end(keyboards), device);

        if (mir::contains(device->capabilities(), mir::input::DeviceCapability::keyboard))
        {
            if (keyboard == end(keyboards))
                add_keyboard(device);
        }
        else
        {
            if (keyboard != end(keyboards))
                keyboards.erase(keyboard);
        }
    }

    void add_keyboard(std::shared_ptr<mir::input::Device> const& keyboard)
    {
        keyboards.push_back(keyboard);
        apply_keymap(keyboard);
    }

#if MIR_VERSION >= MIR_VERSION_NUMBER(0, 24, 1)
    void apply_keymap(std::shared_ptr<mir::input::Device> const& keyboard)
    {
    }
#else
    void apply_keymap(std::shared_ptr<mir::input::Device> const&)
    {
    }
#endif

    void device_removed(std::shared_ptr<mir::input::Device> const& device) override
    {
        if (mir::contains(device->capabilities(), mir::input::DeviceCapability::keyboard))
            keyboards.erase(std::find(begin(keyboards), end(keyboards), device));
    }

    void changes_complete() override
    {
    }

    std::string keymap;
    std::vector<std::shared_ptr<mir::input::Device>> keyboards;
};

miral::Keymap::Keymap(std::string const& keymap) :
    self{std::make_shared<Self>(keymap)}
{
}

miral::Keymap::~Keymap() = default;

miral::Keymap::Keymap(Keymap const&) = default;

auto miral::Keymap::operator=(Keymap const& rhs) -> Keymap& = default;

void miral::Keymap::operator()(mir::Server& server) const
{
    server.add_init_callback([this, &server]
        { server.the_input_device_hub()->add_observer(self); });
}
