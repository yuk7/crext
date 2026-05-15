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

#include "command_handlers.h"

#include <iostream>

#include "commands.h"
#include "format.h"
#include "ext2read.h"

using namespace std;

int command_stat(Ext2File *file, const CommandRequest &request)
{
    if (request.name == "size") {
        cout << file->file_size << endl;
        return 0;
    }
    if (request.name == "mode") {
        cout << mode_str(file->inode.i_mode) << endl;
        return 0;
    }
    if (request.name == "ctime") {
        cout << time_str(file->inode.i_ctime, "%Y-%m-%d %H:%M:%S") << endl;
        return 0;
    }
    if (request.name == "mtime") {
        cout << time_str(file->inode.i_mtime, "%Y-%m-%d %H:%M:%S") << endl;
        return 0;
    }
    if (request.name == "atime" || request.name == "time") {
        cout << time_str(file->inode.i_atime, "%Y-%m-%d %H:%M:%S") << endl;
        return 0;
    }

    return 2;
}
