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

#ifndef CREXT_EXT_DIR_HANDLE_H
#define CREXT_EXT_DIR_HANDLE_H

class Ext2File;
class Ext2Partition;
typedef struct ext2dirent EXT2DIRENT;

class DirHandle {
public:
    DirHandle(Ext2Partition *partition, Ext2File *parent);
    ~DirHandle();

    DirHandle(const DirHandle&) = delete;
    DirHandle& operator=(const DirHandle&) = delete;

    DirHandle(DirHandle&& other) noexcept;
    DirHandle& operator=(DirHandle&& other) noexcept;

    Ext2File *read();
    bool is_open() const;

private:
    void close();

    Ext2Partition *partition;
    EXT2DIRENT *dirent;
};

#endif // CREXT_EXT_DIR_HANDLE_H
