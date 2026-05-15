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
#include "copy.h"
#include "ext2read.h"

using namespace std;

int command_cp(Ext2File *file, const CommandRequest &request)
{
    if (request.ext_path.empty()) {
        cout << "bad parameter" << endl;
        cout << "Source path required." << endl;
        return 1;
    }

    bool success;
    if (EXT2_S_ISDIR(file->inode.i_mode)) {
        success = copy_dir(file, request.local_path);
    } else {
        success = copy_file(file, request.local_path);
    }
    return success ? 0 : 1;
}
