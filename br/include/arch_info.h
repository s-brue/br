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

#ifndef BR_ARCH_INFO_H_
#define BR_ARCH_INFO_H_

#include <vector>
#include <cstdint>
#include <thread>

namespace br {

class cpu_info {
    friend class arch_info;

public:
    cpu_info(unsigned) noexcept;
    [[nodiscard]] unsigned id() const noexcept;

private:
    unsigned cpu_id_;
};

class numa_node_info {
    friend class arch_info;

public:
    [[nodiscard]] const std::vector<cpu_info>& cpus() const noexcept;
    [[nodiscard]] std::uint64_t                mem_total() const noexcept;
    [[nodiscard]] std::uint64_t                mem_free() const noexcept;

private:
    std::vector<cpu_info> cpus_;
    std::uint64_t         mem_total_;
    std::uint64_t         mem_free_;
};

class arch_info {
public:
    arch_info();

    [[nodiscard]] const std::vector<numa_node_info>& numa_nodes() const noexcept;
    [[nodiscard]] unsigned                           number_of_numa_nodes() const noexcept;
    [[nodiscard]] bool                               info_ready() const noexcept;

    static void set_cpu_affinity(std::thread&, unsigned) noexcept;
    static void set_cpu_affinity(std::jthread&, unsigned) noexcept;

    static void set_this_thread_cpu_affinity(unsigned) noexcept;

private:
    std::vector<numa_node_info> numa_nodes_;
    bool                        info_ready_;
};

} // br

#endif //BR_ARCH_INFO_H_
