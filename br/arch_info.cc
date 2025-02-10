//
// Created by brue on 07/02/2025.
//

#include "arch_info.h"

#if defined(_WIN32)
#include <windows.h>
#include <winbase.h>
#include <systemtopologyapi.h>
#endif

#include <iostream>

namespace br {

cpu_info::cpu_info(unsigned cpu_id)
    : cpu_id_{cpu_id}
{
}

unsigned cpu_info::id() const
{
    return cpu_id_;
}

const std::vector<cpu_info>& numa_node_info::cpus() const
{
    return cpus_;
}

arch_info::arch_info()
    : info_ready_{false}
{
#if defined(_WIN32)
    ULONG highest_node_number{0};
    if (!GetNumaHighestNodeNumber(&highest_node_number)) {
        std::cerr << "Cannot get the highest numa node number. Error code: " << GetLastError() << std::endl;
        return;
    }

    ULONGLONG mask{0};
    for (ULONG i = 0; i <= highest_node_number; i++) {
        if (GetNumaNodeProcessorMask(i, &mask)) {
            numa_node_info nn_info;
            unsigned       cpu_id{0};

            while (mask) {
                if (mask & 1) {
                    nn_info.cpus_.emplace_back(cpu_id);
                }
                cpu_id++;
                mask >>= 1;
            }
            nn_info.cpus_.shrink_to_fit();
            numa_nodes_.emplace_back(std::move(nn_info));
        }
        else {
            std::cerr << "Cannot get the numa node processor mask. Error code: " << GetLastError() << std::endl;
            return;
        }
    }

    info_ready_ = true;

#endif // WIN32
}

unsigned arch_info::number_of_numa_nodes() const
{
    return numa_nodes_.size();
}

const std::vector<numa_node_info>& arch_info::numa_nodes() const
{
    return numa_nodes_;
}

bool arch_info::info_ready() const
{
    return info_ready_;
}

} // br
