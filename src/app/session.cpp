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
