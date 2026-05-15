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

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "dir_handle.h"
#include "format.h"
#include "ext2read.h"

using namespace std;

int command_ls(Ext2Partition *partition, Ext2File *file)
{
    if (EXT2_S_ISDIR(file->inode.i_mode)) {
        DirHandle dirent(partition, file);
        while (auto entry = dirent.read()) {
            cout << entry->file_name << endl;
        }
    } else {
        cout << file->file_name << endl;
    }
    return 0;
}

int command_lsl(Ext2Partition *partition, Ext2File *file)
{
    struct FileInfo {
        string name;
        string size;
        string mode;
        string date;
    };
    vector<FileInfo> listfiles;

    size_t fsize_l_max = 0;
    size_t fdate_l_max = 0;

    auto add_file_info = [&](Ext2File* f) {
        FileInfo info;
        info.name = f->file_name;
        info.size = to_string(f->file_size);
        info.mode = mode_str(f->inode.i_mode);
        info.date = time_str(f->inode.i_atime, "%Y-%m-%d %H:%M");

        fsize_l_max = max(fsize_l_max, info.size.length());
        fdate_l_max = max(fdate_l_max, info.date.length());
        listfiles.push_back(info);
    };

    if (EXT2_S_ISDIR(file->inode.i_mode)) {
        DirHandle dirent(partition, file);
        while (auto entry = dirent.read()) {
            add_file_info(entry);
        }
    } else {
        add_file_info(file);
    }

    for (const auto& info : listfiles) {
        cout << info.mode << "  "
             << setw(fsize_l_max) << info.size << "  "
             << setw(fdate_l_max) << info.date << "  "
             << info.name << endl;
    }
    return 0;
}
