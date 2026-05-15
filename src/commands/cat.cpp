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

#include <cstdio>
#include <iostream>
#include <vector>

#if defined(WIN32) || defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

#include "commands.h"
#include "ext2read.h"

using namespace std;

int command_cat(Ext2File *file, const CommandRequest &request)
{
    if (request.ext_path.empty()) {
        cout << "bad parameter" << endl;
        cout << "Source path required." << endl;
        return 1;
    }

    if (!EXT2_S_ISREG(file->inode.i_mode)) {
        cout << "[Skipped  :Not a file] " << "SKIP  " << file->file_name << endl;
        return 1;
    }

#if defined(WIN32) || defined(_WIN32)
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    int blksize = file->partition->get_blocksize();
    vector<char> buffer(blksize);

    lloff_t blocks = file->file_size / blksize;
    lloff_t blkindex;
    for (blkindex = 0; blkindex < blocks; blkindex++) {
        if (file->partition->read_data_block(&file->inode, blkindex, buffer.data()) < 0) {
            return 1;
        }
        cout.write(buffer.data(), blksize);
    }

    int extra = file->file_size % blksize;
    if (extra) {
        if (file->partition->read_data_block(&file->inode, blkindex, buffer.data()) < 0) {
            return 1;
        }
        cout.write(buffer.data(), extra);
    }

    return cout.good() ? 0 : 1;
}
