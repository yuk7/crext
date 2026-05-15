#ifndef CREXT_APP_SESSION_H
#define CREXT_APP_SESSION_H

#include <list>
#include <string>

#include "ext2read.h"

enum class PartitionSelectStatus {
    Selected,
    MultiplePartitions,
    NotFound,
};

class Session {
public:
    explicit Session(bool scan_disks);

    bool open_image(const std::string &path);

    std::list<Ext2Partition *> partitions();
    Ext2Partition *selected_partition() const;
    PartitionSelectStatus select_partition(const std::string &name, bool requested);

private:
    Ext2Read app;
    Ext2Partition *selected;
};

#endif // CREXT_APP_SESSION_H
