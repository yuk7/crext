/**
 * Copyright (C) 2010 by Manish Regmi   (regmi dot manish at gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 **/

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include "platform.h"

int ext2explore_log(const char *str, ...);

FileHandle open_disk(const char *path, int *sect_size)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    *sect_size = 512; // Default sector size
    return fd;
}

int get_ndisks()
{
    // Simplified for now, just return 0 as we don't have a good way to enumerate disks generically
    return 0;
}

void close_disk(FileHandle handle)
{
    if (handle >= 0) {
        close(handle);
    }
}

int read_disk(FileHandle hnd, void *ptr, lloff_t sector, int nsects, int sectorsize)
{
    off_t offset = (off_t)sector * sectorsize;
    if (lseek(hnd, offset, SEEK_SET) == (off_t)-1) {
        return -1;
    }
    return read(hnd, ptr, (size_t)nsects * sectorsize);
}

int write_disk(FileHandle hnd, void *ptr, lloff_t sector, int nsects, int sectorsize)
{
    // Read-only for now
    return -1;
}

int get_nthdevice(char *path, int ndisks)
{
    return -1;
}

#endif
