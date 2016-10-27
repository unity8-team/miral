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

#ifndef MIRAL_COMMAND_LINE_OPTION_H
#define MIRAL_COMMAND_LINE_OPTION_H

#include <mir/optional_value.h>

#include <functional>
#include <memory>
#include <string>

namespace mir { class Server; }

namespace miral
{
/// Add a user configuration option to Mir's option handling.
/// The callback will be invoked during initialisation with a value supplied
/// from the command line, environment variable, config file or the default.
class CommandLineOption
{
public:
    CommandLineOption(
        std::function<void(int value)> callback,
        std::string const& option,
        std::string const& description,
        int default_value);

    CommandLineOption(
        std::function<void(double value)> callback,
        std::string const& option,
        std::string const& description,
        double default_value);

    CommandLineOption(
        std::function<void(std::string const& value)> callback,
        std::string const& option,
        std::string const& description,
        std::string const& default_value);

    CommandLineOption(
        std::function<void(std::string const& value)> callback,
        std::string const& option,
        std::string const& description,
        char const* default_value);

    CommandLineOption(
        std::function<void(bool value)> callback,
        std::string const& option,
        std::string const& description,
        bool default_value);

    CommandLineOption(
        std::function<void(mir::optional_value<int> const& value)> callback,
        std::string const& option,
        std::string const& description);

    CommandLineOption(
        std::function<void(mir::optional_value<std::string> const& value)> callback,
        std::string const& option,
        std::string const& description);

    CommandLineOption(
        std::function<void(mir::optional_value<bool> const& value)> callback,
        std::string const& option,
        std::string const& description);

    CommandLineOption(
        std::function<void(bool is_set)> callback,
        std::string const& option,
        std::string const& description);

    void operator()(mir::Server& server) const;

    ~CommandLineOption();
    CommandLineOption(CommandLineOption const&);
    auto operator=(CommandLineOption const&) -> CommandLineOption&;

private:
    struct Self;
    std::shared_ptr<Self> self;
};
}

#endif //MIRAL_COMMAND_LINE_OPTION_H
