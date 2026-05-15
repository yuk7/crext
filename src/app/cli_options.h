/*
 * Copyright (c) 2015-2026 yuk7
 * Author: yuk7 <yukx00@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CREXT_APP_CLI_OPTIONS_H
#define CREXT_APP_CLI_OPTIONS_H

#include <string>
#include <vector>

struct CliOptions {
    std::string image_path;
    bool list_partitions = false;
    std::string partition_name = "0";
    bool partition_requested = false;
    std::string command;
    bool use_log = false;
    bool show_help = false;
    bool show_version = false;
    std::vector<std::string> args;
};

enum class CliValidationStatus {
    Ok,
    MissingAction,
    ListPartitionsWithOtherOptions,
};

CliOptions parse_cli_options(int argc, char *argv[]);
CliValidationStatus validate_cli_options(const CliOptions &options);

#endif // CREXT_APP_CLI_OPTIONS_H
