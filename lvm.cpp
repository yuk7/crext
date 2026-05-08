/**
 * Ext2read
 * File: lvm.cpp
 **/
/**
 * Copyright (C) 2006 2010 by Manish Regmi   (regmi dot manish at gmail.com)
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

#include <stdlib.h>
#include <sstream>
#include <regex>
#include <algorithm>

#include "lvm.h"


LVM::LVM(FileHandle handle, lloff_t offset, Ext2Read *rd)
{
    pv_handle = handle;
    pv_offset = offset;
    ext2read = rd;
}

LVM::~LVM()
{

}

VolumeGroup *LVM::find_volgroup(const std::string &uuid)
{
    VolumeGroup *grp;
    list<VolumeGroup *>::iterator i;
    list <VolumeGroup *> grouplist = ext2read->get_volgroups();


    for(i = grouplist.begin(); i != grouplist.end(); i++)
    {
        grp = (*i);
        if(grp->uuid.compare(uuid) == 0)
        {
            return grp;
        }
    }

    return NULL;
}

VolumeGroup *LVM::add_volgroup(const std::string &uuid, const std::string &name, int seq, int size)
{
    VolumeGroup *vol;
    //list <VolumeGroup *> grouplist = ext2read->get_volgroups();

    vol = new VolumeGroup(uuid, name, seq, size);
    if(!vol)
        return NULL;

    ext2read->groups.push_back(vol);
    LOG("Volgroup added \n");
    return vol;
}

int LVM::scan_pv()
{
    int ret;
    PV_LABEL_HEADER *header;
    PV_LABEL *label;
    lloff_t sector;
    int length;
    char buffer[SECTOR_SIZE];
    char *metadata;

    for(int i = 0; i < 4; i++)
    {
        sector = pv_offset + i;
        ret = read_disk(pv_handle, buffer, sector, 1, SECTOR_SIZE);

        header = (PV_LABEL_HEADER *) &buffer[0];
        if(strncmp(header->pv_name, "LABELONE", LVM_SIGLEN) != 0)
        {
            continue;
            //LOG("Invalid label. The partition is not LVM2 volume\n");
            //return -1;
        }

        LOG("PV Metadata: %s %UUID=%s offset %d \n",header->pv_name, header->pv_uuid, (int)header->pv_labeloffset);
        strncpy(uuid, header->pv_uuid, UUID_LEN);
        uuid[UUID_LEN] = '\0';
        sector = (header->pv_labeloffset/SECTOR_SIZE) + pv_offset;
        read_disk(pv_handle, buffer, sector, 1, SECTOR_SIZE);
        label = (PV_LABEL *) &buffer[0];

        sector = pv_offset + ((label->pv_offset_low + label->pv_offset_high)/SECTOR_SIZE);
        length = (label->pv_length + SECTOR_SIZE - 1)/SECTOR_SIZE;
        metadata = new char[length * SECTOR_SIZE];
        read_disk(pv_handle, metadata, sector, length, SECTOR_SIZE);

        metadata[label->pv_length] = 0;
        LOG("\n%s", metadata);
        pv_metadata.assign(metadata, label->pv_length);
        parse_metadata();
        delete [] metadata;
        break;
    }
    return 0;
}

// NOTE: Do error checking
int LVM::parse_metadata()
{
    size_t num, num2, numbase;
    std::string volname, suuid;
    int seq = 0, extent_size = 0;
    VolumeGroup *grp;
    std::regex uuid_regex("[a-zA-Z0-9]+(?:-[a-zA-Z0-9]+)+");
    std::smatch match;

    num = pv_metadata.find("{");
    if (num == std::string::npos) return -1;
    volname = pv_metadata.substr(0, num);
    // trim whitespace from volname
    volname.erase(std::remove_if(volname.begin(), volname.end(), [](int c){ return ::isspace(c); }), volname.end());

    if (std::regex_search(pv_metadata, match, uuid_regex)) {
        suuid = match.str();
        suuid.erase(std::remove(suuid.begin(), suuid.end(), '-'), suuid.end());
        num = match.position() + match.length();
    } else {
        num = 0;
    }

    std::regex digit_regex("[0-9]+");
    std::string metadata_sub = pv_metadata.substr(num);
    if (std::regex_search(metadata_sub, match, digit_regex)) {
        seq = std::stoi(match.str());
        num += match.position() + match.length();
        metadata_sub = pv_metadata.substr(num);
        if (std::regex_search(metadata_sub, match, digit_regex)) {
            extent_size = std::stoi(match.str());
        }
    }

    LOG("Volgroup found seq %d, extent_size %d\n", seq, extent_size);
    grp = find_volgroup(suuid);
    if(!grp)
    {
        grp = add_volgroup(suuid, volname, seq, extent_size);
        grp->set_ext2read(ext2read);
    }

    // Parse Physical Volume
    lloff_t dev_size;
    uint32_t pe_start, pe_count;
    num = pv_metadata.find("physical_volumes");
    if(num == std::string::npos)
        return -1;

    std::regex pv_section_regex("pv[0-9\\s\\t]+\\{");
    metadata_sub = pv_metadata.substr(num);
    auto pv_it = std::sregex_iterator(metadata_sub.begin(), metadata_sub.end(), pv_section_regex);
    auto sregex_end = std::sregex_iterator();

    for (; pv_it != sregex_end; ++pv_it) {

        size_t pv_pos = num + pv_it->position();
        std::string pv_sub = pv_metadata.substr(pv_pos);
        if (std::regex_search(pv_sub, match, uuid_regex)) {
            suuid = match.str();
            suuid.erase(std::remove(suuid.begin(), suuid.end(), '-'), suuid.end());
            numbase = pv_pos + match.position() + match.length();
            
            auto find_val = [&](const std::string& key) -> std::string {
                size_t kpos = pv_metadata.find(key, numbase);
                if (kpos == std::string::npos) return "";
                size_t vpos = pv_metadata.find_first_of("0123456789", kpos);
                if (vpos == std::string::npos) return "";
                size_t vend = pv_metadata.find_first_not_of("0123456789", vpos);
                if (vend == std::string::npos) vend = pv_metadata.length();
                return pv_metadata.substr(vpos, vend - vpos);
            };

            std::string val_dev_size = find_val("dev_size");
            std::string val_pe_start = find_val("pe_start");
            std::string val_pe_count = find_val("pe_count");

            if (!val_dev_size.empty()) dev_size = std::stoull(val_dev_size);
            if (!val_pe_start.empty()) pe_start = std::stoul(val_pe_start);
            if (!val_pe_count.empty()) pe_count = std::stoul(val_pe_count);

            LOG("Physical Volume found. start %d, count %d, size %Ld\n", pe_start, pe_count, dev_size);
            PhysicalVolume *pvol;
            pvol = grp->find_physical_volume(suuid);
            if(!pvol && (suuid == uuid))
            {
                pvol = grp->add_physical_volume(suuid, dev_size, pe_start, pe_count, pv_handle, pv_offset);
            }
        }
    }

    // Parse Logical Volume
    int nsegs;
    num = pv_metadata.find("logical_volumes");
    if(num == std::string::npos)
        return -1;

    std::regex lv_section_regex("([a-zA-Z_0-9]+)\\s*\\{");
    metadata_sub = pv_metadata.substr(num);
    auto lv_it = std::sregex_iterator(metadata_sub.begin(), metadata_sub.end(), lv_section_regex);
    
    for (; lv_it != sregex_end; ++lv_it) {
        std::string lvolname_raw = (*lv_it)[1].str();
        if (lvolname_raw == "physical_volumes") continue; // should not happen if we are in logical_volumes section but be safe

        std::string lvolname = volname + "_" + lvolname_raw;
        size_t lv_pos = num + lv_it->position();
        std::string lv_sub = pv_metadata.substr(lv_pos);
        
        if (std::regex_search(lv_sub, match, uuid_regex)) {
            suuid = match.str();
            suuid.erase(std::remove(suuid.begin(), suuid.end(), '-'), suuid.end());
            numbase = lv_pos + match.position() + match.length();

            auto find_val = [&](const std::string& key) -> std::string {
                size_t kpos = pv_metadata.find(key, numbase);
                if (kpos == std::string::npos) return "";
                size_t vpos = pv_metadata.find_first_of("0123456789", kpos);
                if (vpos == std::string::npos) return "";
                size_t vend = pv_metadata.find_first_not_of("0123456789", vpos);
                if (vend == std::string::npos) vend = pv_metadata.length();
                return pv_metadata.substr(vpos, vend - vpos);
            };

            std::string val_nsegs = find_val("segment_count");
            if (!val_nsegs.empty()) nsegs = std::stoi(val_nsegs);
            else nsegs = 0;
            
            LogicalVolume *lvol;
            lvol = grp->find_logical_volume(suuid);
            if(!lvol)
                lvol = grp->add_logical_volume(suuid, nsegs, lvolname);
            LOG("Logical Volume found. Name %s, segments %d\n", lvol->volname.c_str(), nsegs);
            
            size_t seg_base = numbase;
            for(int i = 0; i < nsegs; i++)
            {
                std::regex seg_regex("segment[0-9]+");
                std::string seg_sub = pv_metadata.substr(seg_base);
                if (std::regex_search(seg_sub, match, seg_regex)) {
                    seg_base += match.position() + match.length();
                    
                    auto find_val_seg = [&](const std::string& key) -> std::string {
                        size_t kpos = pv_metadata.find(key, seg_base);
                        if (kpos == std::string::npos) return "";
                        size_t vpos = pv_metadata.find_first_of("0123456789", kpos);
                        if (vpos == std::string::npos) return "";
                        size_t vend = pv_metadata.find_first_not_of("0123456789", vpos);
                        if (vend == std::string::npos) vend = pv_metadata.length();
                        return pv_metadata.substr(vpos, vend - vpos);
                    };

                    std::string val_start = find_val_seg("start_extent");
                    std::string val_count = find_val_seg("extent_count");
                    uint32_t start_extent = val_start.empty() ? 0 : std::stoul(val_start);
                    uint32_t extent_count = val_count.empty() ? 0 : std::stoul(val_count);
                    
                    lv_segment *seg = new lv_segment(start_extent, extent_count);
                    seg->stripe = new struct stripe;
                    
                    std::regex pv_idx_regex("pv[0-9]+");
                    std::string pv_sub = pv_metadata.substr(seg_base);
                    if (std::regex_search(pv_sub, match, pv_idx_regex)) {
                        seg->stripe->stripe_pv = std::stoi(match.str().substr(2));
                        size_t pv_end_pos = seg_base + match.position() + match.length();
                        size_t vpos = pv_metadata.find_first_of("0123456789", pv_end_pos);
                        if (vpos != std::string::npos) {
                            size_t vend = pv_metadata.find_first_not_of("0123456789", vpos);
                            if (vend == std::string::npos) vend = pv_metadata.length();
                            seg->stripe->stripe_start_extent = std::stoul(pv_metadata.substr(vpos, vend - vpos));
                            seg_base = vend;
                        } else {
                            seg->stripe->stripe_start_extent = 0;
                            seg_base = pv_end_pos;
                        }
                    }
                    
                    seg->pvolumes = NULL;
                    lvol->segments.push_back(seg);
                    LOG("Segment found. start %d, count %d\n", start_extent + seg->stripe->stripe_start_extent, extent_count);
                }
            }
        }
    }

    return 0;
}


VolumeGroup::VolumeGroup(const std::string &id, const std::string &name, int seq, int size)
{
    uuid = id;
    volname = name;
    seqno = seq;
    extent_size = size;
}

VolumeGroup::~VolumeGroup()
{
    list<PhysicalVolume *>::iterator i;
    list<LogicalVolume *>::iterator j;
    for(i = pvolumes.begin(); i != pvolumes.end(); i++)
    {
        delete (*i);
    }

    for(j = lvolumes.begin(); j != lvolumes.end(); j++)
    {
        delete (*j);
    }
}

PhysicalVolume *VolumeGroup::find_physical_volume(const std::string &id)
{
    PhysicalVolume *pvol;
    list<PhysicalVolume *>::iterator i;

    for(i = pvolumes.begin(); i != pvolumes.end(); i++)
    {
        pvol = (*i);
        if(pvol->uuid.compare(id) == 0)
        {
            return pvol;
        }
    }

    return NULL;
}

PhysicalVolume *VolumeGroup::add_physical_volume(const std::string &id, lloff_t devsize, uint32_t start, uint32_t count, FileHandle file, lloff_t dsk_offset)
{
    PhysicalVolume *pvol;

    pvol = new PhysicalVolume(id, devsize, start, count, file, dsk_offset);
    if(!pvol)
        return NULL;

    pvolumes.push_back(pvol);
    return pvol;
}

LogicalVolume *VolumeGroup::find_logical_volume(const std::string &id)
{
    LogicalVolume *lvol;
    list<LogicalVolume *>::iterator i;

    for(i = lvolumes.begin(); i != lvolumes.end(); i++)
    {
        lvol = (*i);
        if(lvol->uuid.compare(id) == 0)
        {
            return lvol;
        }
    }

    return NULL;
}

LogicalVolume *VolumeGroup::add_logical_volume(const std::string &id, int count, const std::string &vname)
{
    LogicalVolume *lvol;

    lvol = new LogicalVolume(id, count, vname, this);
    if(!lvol)
        return NULL;

    lvolumes.push_back(lvol);
    return lvol;
}

void VolumeGroup::logical_mount()
{
    //EXT2_SUPER_BLOCK sblock;
    LogicalVolume *lvol;
    //PhysicalVolume *pvol;
    Ext2Partition *partition;
    lv_segment *seg;
    lv_segment *root = NULL;
    list<LogicalVolume *>::iterator i;
    list<lv_segment *>::iterator j;
    list<PhysicalVolume *>::iterator k;
    lloff_t start;
    int index = 0;

    LOG("Mouning Logical VOLUMES\n");
    for(i = lvolumes.begin(); i != lvolumes.end(); i++)
    {
        lvol = (*i);
        for(j = lvol->segments.begin(); j != lvol->segments.end(); j++)
        {
            seg = (*j);
            if(seg->start_extent == 0)
                root = seg;

            index = 0;
            for(k = pvolumes.begin(); k != pvolumes.end(); k++)
            {
                if(seg->stripe->stripe_pv == index)
                {
                    seg->pvolumes = (*k);
                    break;
                }
                index++;
            }
        }

        if(!root)
            continue;

        int off = ((root->start_extent + root->stripe->stripe_start_extent) * lvol->this_group->extent_size);
        LOG("PE start %d offset %d extoff %d B=%d A=%d %s\n", (int)root->pvolumes->pe_start,
            (int)root->pvolumes->offset, lvol->this_group->extent_size, (int)root->start_extent, (int)root->stripe->stripe_start_extent, lvol->volname.c_str());
        start = root->pvolumes->pe_start + root->pvolumes->offset + off;
        partition = new Ext2Partition(root->pvolumes->dev_size, start, SECTOR_SIZE, root->pvolumes->handle, lvol);
        if(partition->is_valid)
        {
            partition->set_image_name(lvol->volname.c_str());
            LOG("adding %s\n", partition->get_linux_name().c_str());
            ext2read->add_partition(partition);
        }
        else
        {
            LOG("Invalid Partition %s\n", partition->get_linux_name().c_str());
            delete partition;
        }
    }
}

PhysicalVolume::PhysicalVolume(const std::string &id, lloff_t devsize, uint32_t start, uint32_t count, FileHandle file, lloff_t dsk_offset)
{
    uuid = id;
    dev_size = devsize;
    pe_start = start;
    pe_count = count;
    handle = file;
    offset = dsk_offset;
}

LogicalVolume::LogicalVolume(const std::string &id, int nsegs, const std::string &vname, VolumeGroup *vol)
{
    uuid = id;
    segment_count = nsegs;
    this_group = vol;
    volname = vname;
}

LogicalVolume::~LogicalVolume()
{

}

lloff_t LogicalVolume::lvm_mapper(lloff_t sectno)
{
    lv_segment *seg;
    lloff_t sect_mapped = 0;
    uint32_t extent_no, extent_offset;
    list<lv_segment *>::iterator iterate;

    extent_no = sectno / this_group->extent_size;
    extent_offset = sectno % this_group->extent_size;

    for(iterate = segments.begin(); iterate != segments.end(); iterate++)
    {
        seg = (*iterate);
        if((extent_no >= seg->start_extent) && (extent_no < (seg->start_extent + seg->extent_count)))
        {
            sect_mapped = (lloff_t)(extent_no *  this_group->extent_size) + extent_offset;
            sect_mapped += seg->pvolumes->pe_start + seg->pvolumes->offset + seg->stripe->stripe_start_extent;
            break;
        }
    }

    if(sect_mapped == 0)
    {
        LOG("Error in LVM Mapping \n");
    }
    return sect_mapped;
}
