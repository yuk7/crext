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
#include <fstream>
#include <filesystem>
#include <time.h>
#include <cstring>
#include "lvm.h"
#include "ext2read.h"
#include "ext2fs.h"

using namespace std;
namespace fs = std::filesystem;

string mode_str(uint16_t mode);
string time_str(uint32_t time, const char *format);
bool copy_dir(Ext2File *srcfile, const string &destdir);
bool copy_file(Ext2File *srcfile, string destfile);
bool show_progress(int now, int max, const string &str);

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
            if (i + 1 < argc) optsetpart = argv[++i];
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

    Ext2Read *app = new Ext2Read();

    if (!openfopt.empty()) {
        int result = app->add_loopback(openfopt.c_str());
        if (result <= 0) {
            cout << "Open image file failed." << endl;
            LOG("No valid Ext2 Partitions found in the disk image.");
            return 1;
        }
    }

    list<Ext2Partition *> parts = app->get_partitions();

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

    Ext2Partition *setpart = nullptr;
    bool spsetd = false;
    for (auto part : parts) {
        if (optsetpart == "0") {
            if (part->get_linux_name().find("/dev/sd") == string::npos) {
                setpart = part;
                spsetd = true;
            }
        } else {
            if (part->get_linux_name().find(optsetpart) != string::npos) {
                setpart = part;
                spsetd = true;
            }
        }
    }

    if (!spsetd) {
        cout << "ERR:can't set partition." << endl;
        cout << "*Please make sure ext partitions name exist." << endl;
        return 1;
    }

    string optepath = "";
    string optlpath = "";
    if (pargs.size() >= 1) optepath = pargs[0];
    if (pargs.size() >= 2) optlpath = pargs[1];

    vector<string> epathlist;
    {
        stringstream ss(optepath);
        string item;
        while (getline(ss, item, '/')) {
            if (!item.empty()) epathlist.push_back(item);
        }
    }

    Ext2File *ptr = setpart->get_root();
    ext2dirent *dirent = setpart->open_dir(ptr);
    Ext2File *setefile = ptr;

    for (const auto& path : epathlist) {
        bool efsetd = false;
        while ((ptr = setpart->read_dir(dirent)) != nullptr) {
            if (ptr->file_name == path) {
                setefile = ptr;
                efsetd = true;
                break;
            }
        }
        if (!efsetd) {
            cout << "ERR:Ext Path Not found" << endl;
            cout << "*Please make sure path in selected ext partition exist." << endl;
            return 1;
        } else {
            dirent = setpart->open_dir(setefile);
        }
    }

    if (optcmd == "ls") {
        if (EXT2_S_ISDIR(setefile->inode.i_mode)) {
            ext2dirent *lsdirent = setpart->open_dir(setefile);
            while (auto entry = setpart->read_dir(lsdirent)) {
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
            ext2dirent *lsdirent = setpart->open_dir(setefile);
            while (auto entry = setpart->read_dir(lsdirent)) {
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
        if (!optepath.empty()) {
            if (EXT2_S_ISDIR(setefile->inode.i_mode)) {
                copy_dir(setefile, optlpath);
            } else {
                copy_file(setefile, optlpath);
            }
        }
        return 0;
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


string mode_str(uint16_t mode)
{
    string str = "";
    if (EXT2_S_ISREG(mode)) str += "-";
    else if (EXT2_S_ISDIR(mode)) str += "d";
    else if (EXT2_S_ISLINK(mode)) str += "l";
    else str += "?";

    str += (mode & EXT2_S_IRUSR) ? "r" : "-";
    str += (mode & EXT2_S_IWUSR) ? "w" : "-";
    str += (mode & EXT2_S_IXUSR) ? "x" : "-";
    str += (mode & EXT2_S_IRGRP) ? "r" : "-";
    str += (mode & EXT2_S_IWGRP) ? "w" : "-";
    str += (mode & EXT2_S_IXGRP) ? "x" : "-";
    str += (mode & EXT2_S_IROTH) ? "r" : "-";
    str += (mode & EXT2_S_IWOTH) ? "w" : "-";
    str += (mode & EXT2_S_IXOTH) ? "x" : "-";
    return str;
}

string time_str(uint32_t time, const char *format)
{
    char str[256];
    time_t timet = time;
    struct tm *tm = localtime(&timet);
    if (!tm) return "N/A";
    strftime(str, 255, format, tm);
    return string(str);
}

bool copy_dir(Ext2File *srcfile, const string &destdir)
{
    Ext2Partition *part = srcfile->partition;
    ext2dirent *dirent = part->open_dir(srcfile);

    fs::create_directories(destdir);

    Ext2File *entry;
    while ((entry = part->read_dir(dirent)) != nullptr) {
        string cdestpath = (fs::path(destdir) / entry->file_name).string();
        if (EXT2_S_ISDIR(entry->inode.i_mode)) {
            copy_dir(entry, cdestpath);
        } else {
            copy_file(entry, cdestpath);
        }
    }
    return true;
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
