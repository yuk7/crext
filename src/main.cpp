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

#include <iostream>
#include <string>
#include "cli_options.h"
#include "commands.h"
#include "session.h"
#include "ext2read.h"

using namespace std;

void show_help() {
    cout << "crext is command-line based ext image/partition reader." << endl;
    cout << "Usage: crext [options] [ePath] [lPath]" << endl;
    cout << "Options:" << endl;
    cout << "  -f, --fopen <ImgFilePath>  Open Image File" << endl;
    cout << "  -l, --lp                   List Partitions" << endl;
    cout << "  -s, --sp <Partition name>  Set Partition (default: 0)" << endl;
    cout << "  -c, --cmd <Command>        ls|lsl|cat|cp|size|mode|ctime|mtime|atime" << endl;
    cout << "  --log                      Write log to file" << endl;
    cout << "  --help                     Show this help" << endl;
    cout << "  --version                  Show version" << endl;
}

int main(int argc, char *argv[])
{
    CliOptions options = parse_cli_options(argc, argv);

    if (options.show_help) {
        show_help();
        return 0;
    }

    if (options.show_version) {
        cout << "crext version 2.6.0b" << endl;
        return 0;
    }

    if (options.use_log)
        log_init();

    CliValidationStatus validation = validate_cli_options(options);
    if (validation == CliValidationStatus::MissingAction) {
        cout << "bad parameter" << endl;
        cout << "List partition or Command option required." << endl << endl;
        show_help();
        return 1;
    }

    if (validation == CliValidationStatus::ListPartitionsWithOtherOptions) {
        cout << "bad parameter" << endl;
        cout << "List partitions option cannot use with Other options" << endl << endl;
        show_help();
        return 1;
    }

    Session session(options.image_path.empty());

    if (!options.image_path.empty()) {
        if (!session.open_image(options.image_path)) {
            cout << "Open image file failed." << endl;
            LOG("No valid Ext2 Partitions found in the disk image.");
            return 1;
        }
    }

    list<Ext2Partition *> parts = session.partitions();

    if (parts.empty()) {
        cout << "ERR:No partitions detected." << endl;
        cout << "Reading disk is required an Administrator. (not required for image file)" << endl << endl;
        cout << "*Please make sure ext partitions exists." << endl;
        cout << "*Please make sure you are running this application as an Administrator." << endl;
        return 1;
    }

    if (options.list_partitions) {
        for (auto part : parts) {
            cout << part->get_linux_name() << endl;
        }
        return 0;
    }

    PartitionSelectStatus select_status = session.select_partition(options.partition_name, options.partition_requested);
    if (select_status == PartitionSelectStatus::MultiplePartitions) {
        cout << "ERR:Multiple ext partitions detected." << endl;
        cout << "*Please select a partition with -s/--sp." << endl;
        cout << "*Available partitions:" << endl;
        for (auto part : parts) {
            cout << part->get_linux_name() << endl;
        }
        return 1;
    }

    if (select_status == PartitionSelectStatus::NotFound) {
        cout << "ERR:can't set partition." << endl;
        cout << "*Please make sure ext partitions name exist." << endl;
        return 1;
    }

    Ext2Partition *setpart = session.selected_partition();

    CommandRequest request;
    request.name = options.command;
    if (options.args.size() >= 1) request.ext_path = options.args[0];
    if (options.args.size() >= 2) request.local_path = options.args[1];

    int result = execute_command(setpart, request);
    if (result == 2) {
        show_help();
        return 0;
    }
    return result;
}
