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

#include "session.h"

#include <sstream>
#include <vector>

#include "ext_path.h"

using namespace std;

namespace {

vector<string> split_path(const string &path)
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

string join_absolute_path(const vector<string> &parts)
{
    if (parts.empty()) {
        return "/";
    }

    string result;
    for (const auto &part : parts) {
        result += "/";
        result += part;
    }
    return result;
}

bool is_dot_parent_shorthand(const string &part)
{
    if (part.size() < 3) {
        return false;
    }

    for (char ch : part) {
        if (ch != '.') {
            return false;
        }
    }

    return true;
}

} // namespace

Session::Session(bool scan_disks)
    : app(scan_disks), selected(nullptr), current_working_directory("/")
{
}

bool Session::open_image(const string &path)
{
    return app.add_loopback(path.c_str()) > 0;
}

list<Ext2Partition *> Session::partitions()
{
    return app.get_partitions();
}

Ext2Partition *Session::selected_partition() const
{
    return selected;
}

PartitionSelectStatus Session::select_partition(const string &name, bool requested)
{
    list<Ext2Partition *> parts = partitions();
    selected = nullptr;

    if (!requested && parts.size() == 1) {
        selected = parts.front();
    }

    if (!requested && parts.size() > 1) {
        return PartitionSelectStatus::MultiplePartitions;
    }

    for (auto part : parts) {
        if (!requested && name == "0") {
            if (part->get_linux_name().find("/dev/sd") == string::npos) {
                selected = part;
            }
        } else if (requested) {
            if (part->get_linux_name().find(name) != string::npos) {
                selected = part;
            }
        }
    }

    if (!selected) {
        return PartitionSelectStatus::NotFound;
    }

    return PartitionSelectStatus::Selected;
}

const string &Session::cwd() const
{
    return current_working_directory;
}

string Session::resolve_shell_path(const string &path) const
{
    vector<string> parts;

    if (path.empty()) {
        return current_working_directory;
    }

    if (path[0] != '/') {
        parts = split_path(current_working_directory);
    }

    for (const auto &part : split_path(path)) {
        if (part == ".") {
            continue;
        }
        if (part == "..") {
            if (!parts.empty()) {
                parts.pop_back();
            }
            continue;
        }
        if (is_dot_parent_shorthand(part)) {
            for (size_t i = 1; i < part.size() && !parts.empty(); ++i) {
                parts.pop_back();
            }
            continue;
        }
        parts.push_back(part);
    }

    return join_absolute_path(parts);
}

bool Session::set_cwd(const string &path)
{
    string resolved = resolve_shell_path(path);
    Ext2File *file = resolve_ext_path(selected, resolved);
    if (!file) {
        return false;
    }

    if (!EXT2_S_ISDIR(file->inode.i_mode)) {
        return false;
    }

    current_working_directory = resolved;
    return true;
}
