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

#include "ext_path.h"

#include <sstream>
#include <vector>

#include "dir_handle.h"
#include "ext2read.h"

using namespace std;

namespace {

vector<string> split_ext_path(const string &path)
{
    vector<string> parts;
    stringstream ss(path);
    string item;

    while (getline(ss, item, '/')) {
        if (!item.empty()) {
            parts.push_back(item);
        }
    }

    return parts;
}

} // namespace

Ext2File *resolve_ext_path(Ext2Partition *partition, const string &path)
{
    if (!partition) {
        return nullptr;
    }

    Ext2File *current = partition->get_root();

    for (const auto& part : split_ext_path(path)) {
        bool found = false;
        DirHandle dirent(partition, current);
        Ext2File *entry;

        while ((entry = dirent.read()) != nullptr) {
            if (entry->file_name == part) {
                current = entry;
                found = true;
                break;
            }
        }

        if (!found) {
            return nullptr;
        }
    }

    return current;
}
