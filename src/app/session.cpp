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

using namespace std;

Session::Session(bool scan_disks)
    : app(scan_disks), selected(nullptr)
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
