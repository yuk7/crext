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

#include "format.h"

#include <ctime>

#include "ext2read.h"

using namespace std;

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
