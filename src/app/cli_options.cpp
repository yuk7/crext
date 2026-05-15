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

#include "cli_options.h"

using namespace std;

CliOptions parse_cli_options(int argc, char *argv[])
{
    CliOptions options;

    if (argc == 1) {
        options.show_help = true;
        return options;
    }

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-f" || arg == "--fopen") {
            if (i + 1 < argc) options.image_path = argv[++i];
        } else if (arg == "-l" || arg == "--lp") {
            options.list_partitions = true;
        } else if (arg == "-s" || arg == "--sp") {
            if (i + 1 < argc) {
                options.partition_name = argv[++i];
                options.partition_requested = true;
            }
        } else if (arg == "-c" || arg == "--cmd") {
            if (i + 1 < argc) options.command = argv[++i];
        } else if (arg == "--log") {
            options.use_log = true;
        } else if (arg == "--help") {
            options.show_help = true;
        } else if (arg == "--version") {
            options.show_version = true;
        } else if (arg.size() > 0 && arg[0] != '-') {
            options.args.push_back(arg);
        }
    }

    return options;
}

CliValidationStatus validate_cli_options(const CliOptions &options)
{
    if (!(options.list_partitions || !options.command.empty())) {
        return CliValidationStatus::MissingAction;
    }

    if (options.list_partitions && (!options.command.empty() || !options.args.empty())) {
        return CliValidationStatus::ListPartitionsWithOtherOptions;
    }

    return CliValidationStatus::Ok;
}
