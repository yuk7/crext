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

#include "commands.h"

#include <iostream>

#include "command_handlers.h"
#include "ext_path.h"
#include "ext2read.h"

using namespace std;

int execute_command(Ext2Partition *partition, const CommandRequest &request)
{
    Ext2File *file = resolve_ext_path(partition, request.ext_path);
    if (!file) {
        cout << "ERR:Ext Path Not found" << endl;
        cout << "*Please make sure path in selected ext partition exist." << endl;
        return 1;
    }

    if (request.name == "ls") {
        return command_ls(partition, file);
    }

    if (request.name == "lsl") {
        return command_lsl(partition, file);
    }

    if (request.name == "cat") {
        return command_cat(file, request);
    }

    if (request.name == "cp") {
        return command_cp(file, request);
    }

    int stat_result = command_stat(file, request);
    if (stat_result != 2) {
        return stat_result;
    }

    cout << "bad parameter" << endl;
    cout << "The specified command option does not exist." << endl << endl;
    return 2;
}
