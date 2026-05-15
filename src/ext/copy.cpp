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

#include "copy.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "dir_handle.h"
#include "ext2read.h"

using namespace std;
namespace fs = std::filesystem;

namespace {

bool show_progress(int now, int max, const string &str)
{
    int iprog = (max == 0) ? 100 : (int)((double)now / (double)max * 100);
    if (iprog > 100) iprog = 100;
    string progstr = "[";
    for (int i = 0; i < (iprog / 5); i++) progstr += "=";
    for (int i = (iprog / 5); i < 20; i++) progstr += " ";
    progstr += "]";

    cout << progstr << " " << setw(3) << iprog << "%  " << str << "\r" << flush;
    return true;
}

} // namespace

bool copy_dir(Ext2File *srcfile, const string &destdir)
{
    string targetdir = destdir.empty() ? srcfile->file_name : destdir;
    Ext2Partition *part = srcfile->partition;
    DirHandle dirent(part, srcfile);

    error_code ec;
    if (!fs::create_directories(targetdir, ec) && ec) {
        LOG("Error creating directory %s.\n", targetdir.c_str());
        return false;
    }

    bool success = true;
    Ext2File *entry;
    while ((entry = dirent.read()) != nullptr) {
        string cdestpath = (fs::path(targetdir) / entry->file_name).string();
        if (EXT2_S_ISDIR(entry->inode.i_mode)) {
            success = copy_dir(entry, cdestpath) && success;
        } else {
            success = copy_file(entry, cdestpath) && success;
        }
    }
    return success;
}

bool copy_file(Ext2File *srcfile, string destfile)
{
    if (destfile.empty() || fs::is_directory(destfile)) {
        destfile = (fs::path(destfile) / srcfile->file_name).string();
    }

    if (!EXT2_S_ISREG(srcfile->inode.i_mode)) {
        cout << "[Skipped  :Not a file] " << "SKIP  " << destfile << endl;
        return false;
    }

    int blksize = srcfile->partition->get_blocksize();
    vector<char> buffer(blksize);

    ofstream filesav(destfile, ios::binary | ios::trunc);
    if (!filesav.is_open()) {
        LOG("Error creating file %s.\n", srcfile->file_name.c_str());
        return false;
    }

    lloff_t blocks = srcfile->file_size / blksize;
    lloff_t blkindex;
    for (blkindex = 0; blkindex < blocks; blkindex++) {
        if (srcfile->partition->read_data_block(&srcfile->inode, blkindex, buffer.data()) < 0) {
            return false;
        }
        filesav.write(buffer.data(), blksize);
        show_progress(blkindex, blocks, destfile);
    }

    int extra = srcfile->file_size % blksize;
    if (extra) {
        if (srcfile->partition->read_data_block(&srcfile->inode, blkindex, buffer.data()) < 0) {
            return false;
        }
        filesav.write(buffer.data(), extra);
    }
    filesav.close();
    show_progress(1, 1, destfile);
    cout << endl;
    return true;
}
