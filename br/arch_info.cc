// MIT License
//
// Copyright (c) 2025 Sergio Pérez Camacho
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "arch_info.h"

#if defined(_WIN32)

#include <windows.h>
#include <winbase.h>
#include <systemtopologyapi.h>

#elif defined(__linux__)

#include <fstream>
#include <filesystem>
#include <regex>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>

#endif

#if defined(__MINGW32__)
#include <pthread.h>
#endif

#include <iostream>

namespace br {

cpu_info::cpu_info(unsigned cpu_id) noexcept
    : cpu_id_{cpu_id}
{
}

unsigned cpu_info::id() const noexcept
{
    return cpu_id_;
}

const std::vector<cpu_info>& numa_node_info::cpus() const noexcept
{
    return cpus_;
}

std::uint64_t numa_node_info::mem_total() const noexcept
{
    return mem_total_;
}

std::uint64_t numa_node_info::mem_free() const noexcept
{
    return mem_free_;
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

#elif defined(__linux__)
    const std::string nodes_path = "/sys/devices/system/node/";
    const std::regex  re("^cpu([0-9]+)$");

    try {
        std::cmatch m;
        for (const auto& entry_node : std::filesystem::directory_iterator(nodes_path)) {
            const std::string& dir_name = entry_node.path().filename().string();
            if (entry_node.is_directory() && dir_name.find("node") == 0) {
                numa_node_info nn_info;
                for (const auto& entry_cpu : std::filesystem::directory_iterator(nodes_path + dir_name)) {
                    const std::string& file_name = entry_cpu.path().filename().string();
                    if (entry_node.is_directory() && std::regex_match(file_name.c_str(), m, re)) {
                        nn_info.cpus_.emplace_back(stoi(m[1]));
                    }
                }
                std::ifstream mem_info(nodes_path + dir_name + "/meminfo");
                if (mem_info.is_open()) {
                    std::string line;
                    while (std::getline(mem_info, line)) {
                        const auto line_right = line.substr(line.find(':') + 1);
                        if (line.find("MemTotal:") != std::string::npos) {
                            nn_info.mem_total_ = std::stoull(line_right);
                        }
                        else if (line.find("MemFree") != std::string::npos) {
                            nn_info.mem_free_ = std::stoull(line_right);
                        }
                    }
                }
                nn_info.cpus_.shrink_to_fit();
                numa_nodes_.emplace_back(std::move(nn_info));
            }
        }
        info_ready_ = true;
    }
    catch (std::exception& e) {
        std::cerr << "Cannot get the numa info: " << e.what() << std::endl;
    }
#endif
}

unsigned arch_info::number_of_numa_nodes() const noexcept
{
    return numa_nodes_.size();
}

const std::vector<numa_node_info>& arch_info::numa_nodes() const noexcept
{
    return numa_nodes_;
}

bool arch_info::info_ready() const noexcept
{
    return info_ready_;
}

namespace {
#if defined(_WIN32)
void _set_cpu_affinity(HANDLE t, unsigned cpu_id) noexcept
{
    const DWORD_PTR cpu_mask = (1 << cpu_id);
    if (SetThreadAffinityMask(t, cpu_mask) == 0) {
        std::cerr << "Error: Could change thread affinity for CPU " << cpu_id << std::endl;
    }
}
#if defined(__MINGW32__)
void _set_cpu_affinity(pthread_t t, unsigned cpu_id) noexcept
{
    const DWORD threadId = GetThreadId(reinterpret_cast<HANDLE>(t));
    if (!threadId) {
        std::cerr << "Error: Cannot obtain thread id. Could change thread affinity for CPU " << cpu_id << std::endl;
        return;
    }

    HANDLE ht = OpenThread(THREAD_ALL_ACCESS, FALSE, threadId);
    if (!ht) {
        std::cerr << "Error: Could change thread affinity for CPU " << cpu_id << std::endl;
    }
    const DWORD_PTR cpu_mask = (1 << cpu_id);
    SetThreadAffinityMask(ht, cpu_mask);
    CloseHandle(ht);
}

#endif

#elif defined(__linux__)
void _set_cpu_affinity(pthread_t t, unsigned cpu_id) noexcept
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);

    if (pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpuset) != 0) {
        std::cerr << "Error: Could change thread affinity for CPU " << cpu_id << std::endl;
    }
}

void _set_cpu_affinity(pid_t pid, unsigned cpu_id) noexcept
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);

    if (sched_setaffinity(pid, sizeof(cpuset), &cpuset) != -1) {
        std::cerr << "Error: Could change process affinity for CPU " << cpu_id << std::endl;
    }
}
#endif

} // namespace

void arch_info::set_cpu_affinity(std::thread& th, unsigned cpu_id) noexcept
{
#if defined(__MINGW32__)
    _set_cpu_affinity(th.native_handle(), cpu_id);
#elif defined(__linux__)
    _set_cpu_affinity(th.native_handle(), cpu_id);
#endif
}

void arch_info::set_cpu_affinity(std::jthread& jth, unsigned cpu_id) noexcept
{
#if defined(__MINGW32__)
    _set_cpu_affinity(jth.native_handle(), cpu_id);
#elif defined(__linux__)
    _set_cpu_affinity(jth.native_handle(), cpu_id);
#endif
}

void arch_info::set_this_thread_cpu_affinity(unsigned cpu_id) noexcept
{
#if defined(_WIN32)
    _set_cpu_affinity(GetCurrentThread(), cpu_id);
#elif defined(__linux__)
    _set_cpu_affinity(pthread_self(), cpu_id);
#endif
}



} // br
