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
#include <iomanip>
#include <vector>
#include <string>
#include <time.h>
#include <cstring>
#include "copy.h"
#include "dir_handle.h"
#include "ext_path.h"
#include "format.h"
#include "session.h"
#include "ext2read.h"
#include "ext2fs.h"

using namespace std;

void show_help() {
    cout << "crext is command-line based ext image/partition reader." << endl;
    cout << "Usage: crext [options] [ePath] [lPath]" << endl;
    cout << "Options:" << endl;
    cout << "  -f, --fopen <ImgFilePath>  Open Image File" << endl;
    cout << "  -l, --lp                   List Partitions" << endl;
    cout << "  -s, --sp <Partition name>  Set Partition (default: 0)" << endl;
    cout << "  -c, --cmd <Command>        ls|lsl|cp|size|mode|ctime|mtime|atime" << endl;
    cout << "  --log                      Write log to file" << endl;
    cout << "  --help                     Show this help" << endl;
    cout << "  --version                  Show version" << endl;
}

int main(int argc, char *argv[])
{
    string openfopt = "";
    bool listpart = false;
    string optsetpart = "0";
    bool setpart_requested = false;
    string optcmd = "";
    bool use_log = false;
    vector<string> pargs;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-f" || arg == "--fopen") {
            if (i + 1 < argc) openfopt = argv[++i];
        } else if (arg == "-l" || arg == "--lp") {
            listpart = true;
        } else if (arg == "-s" || arg == "--sp") {
            if (i + 1 < argc) {
                optsetpart = argv[++i];
                setpart_requested = true;
            }
        } else if (arg == "-c" || arg == "--cmd") {
            if (i + 1 < argc) optcmd = argv[++i];
        } else if (arg == "--log") {
            use_log = true;
        } else if (arg == "--help") {
            show_help();
            return 0;
        } else if (arg == "--version") {
            cout << "crext version 2.6.0b" << endl;
            return 0;
        } else if (arg.size() > 0 && arg[0] != '-') {
            pargs.push_back(arg);
        }
    }

    if (argc == 1) {
        show_help();
        return 0;
    }

    if (use_log)
        log_init();

    if (!(listpart || !optcmd.empty())) {
        cout << "bad parameter" << endl;
        cout << "List partition or Command option required." << endl << endl;
        show_help();
        return 1;
    }

    if (listpart && ((!optsetpart.empty() && optsetpart != "0") || !optcmd.empty() || !pargs.empty())) {
        // Allow listpart with default optsetpart="0"
        if (!optcmd.empty() || !pargs.empty()) {
            cout << "bad parameter" << endl;
            cout << "List partitions option cannot use with Other options" << endl << endl;
            show_help();
            return 1;
        }
    }

    Session session(openfopt.empty());

    if (!openfopt.empty()) {
        if (!session.open_image(openfopt)) {
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

    if (listpart) {
        for (auto part : parts) {
            cout << part->get_linux_name() << endl;
        }
        return 0;
    }

    PartitionSelectStatus select_status = session.select_partition(optsetpart, setpart_requested);
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

    string optepath = "";
    string optlpath = "";
    if (pargs.size() >= 1) optepath = pargs[0];
    if (pargs.size() >= 2) optlpath = pargs[1];

    Ext2File *setefile = resolve_ext_path(setpart, optepath);
    if (!setefile) {
        cout << "ERR:Ext Path Not found" << endl;
        cout << "*Please make sure path in selected ext partition exist." << endl;
        return 1;
    }

    if (optcmd == "ls") {
        if (EXT2_S_ISDIR(setefile->inode.i_mode)) {
            DirHandle lsdirent(setpart, setefile);
            while (auto entry = lsdirent.read()) {
                cout << entry->file_name << endl;
            }
        } else {
            cout << setefile->file_name << endl;
        }
        return 0;
    }

    if (optcmd == "lsl") {
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

        if (EXT2_S_ISDIR(setefile->inode.i_mode)) {
            DirHandle lsdirent(setpart, setefile);
            while (auto entry = lsdirent.read()) {
                add_file_info(entry);
            }
        } else {
            add_file_info(setefile);
        }

        for (const auto& info : listfiles) {
            cout << info.mode << "  "
                 << setw(fsize_l_max) << info.size << "  "
                 << setw(fdate_l_max) << info.date << "  "
                 << info.name << endl;
        }
        return 0;
    }

    if (optcmd == "cp") {
        if (optepath.empty()) {
            cout << "bad parameter" << endl;
            cout << "Source path required." << endl;
            return 1;
        }
        bool success;
        if (EXT2_S_ISDIR(setefile->inode.i_mode)) {
            success = copy_dir(setefile, optlpath);
        } else {
            success = copy_file(setefile, optlpath);
        }
        return success ? 0 : 1;
    }

    if (optcmd == "size") {
        cout << setefile->file_size << endl;
        return 0;
    }
    if (optcmd == "mode") {
        cout << mode_str(setefile->inode.i_mode) << endl;
        return 0;
    }
    if (optcmd == "ctime") {
        cout << time_str(setefile->inode.i_ctime, "%Y-%m-%d %H:%M:%S") << endl;
        return 0;
    }
    if (optcmd == "mtime") {
        cout << time_str(setefile->inode.i_mtime, "%Y-%m-%d %H:%M:%S") << endl;
        return 0;
    }
    if (optcmd == "atime" || optcmd == "time") {
        cout << time_str(setefile->inode.i_atime, "%Y-%m-%d %H:%M:%S") << endl;
        return 0;
    }

    cout << "bad parameter" << endl;
    cout << "The specified command option does not exist." << endl << endl;
    show_help();

    return 0;
}
