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

#include "dir_handle.h"

#include "ext2read.h"

DirHandle::DirHandle(Ext2Partition *partition, Ext2File *parent)
    : partition(partition), dirent(nullptr)
{
    if (partition && parent) {
        dirent = partition->open_dir(parent);
    }
}

DirHandle::~DirHandle()
{
    close();
}

DirHandle::DirHandle(DirHandle&& other) noexcept
    : partition(other.partition), dirent(other.dirent)
{
    other.partition = nullptr;
    other.dirent = nullptr;
}

DirHandle& DirHandle::operator=(DirHandle&& other) noexcept
{
    if (this != &other) {
        close();
        partition = other.partition;
        dirent = other.dirent;
        other.partition = nullptr;
        other.dirent = nullptr;
    }
    return *this;
}

Ext2File *DirHandle::read()
{
    if (!partition || !dirent) {
        return nullptr;
    }
    return partition->read_dir(dirent);
}

bool DirHandle::is_open() const
{
    return dirent != nullptr;
}

void DirHandle::close()
{
    if (partition && dirent) {
        partition->close_dir(dirent);
    }
    partition = nullptr;
    dirent = nullptr;
}
