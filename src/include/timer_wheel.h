// MIT License
//
// Copyright (c) 2024 Sergio Pérez Camacho
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

#ifndef BR_TIMER_WHEEL_H_
#define BR_TIMER_WHEEL_H_

#include "ilist.h"

#include <chrono>

namespace br {
template <typename MUTEX_LOCK=detail_::void_mutex>
class basic_expirable;

using expirable = basic_expirable<>;
using ts_expirable = basic_expirable<std::mutex>;

template <typename MUTEX_LOCK>
using basic_expirables_list = ilist<basic_expirable<MUTEX_LOCK>, MUTEX_LOCK>;

template <typename MUTEX_LOCK=detail_::void_mutex>
class basic_timer_wheel;

using timer_wheel = basic_timer_wheel<>;
using ts_timer_wheel = basic_timer_wheel<std::mutex>;

template <typename MUTEX_LOCK>
class basic_expirable: public basic_expirables_list<MUTEX_LOCK>::node {
    friend class basic_timer_wheel<MUTEX_LOCK>;

public:
    basic_expirable() noexcept                            = default;
    basic_expirable(const basic_expirable&) noexcept            = default;
    basic_expirable& operator=(const basic_expirable&) noexcept = default;
    basic_expirable(basic_expirable&&) noexcept                 = default;
    basic_expirable& operator=(basic_expirable&&) noexcept      = default;
    virtual    ~basic_expirable()                         = default;

    virtual void expire() = 0;

private:
    std::size_t n_loops_{0};
};

template <typename MUTEX_LOCK>
class basic_timer_wheel {
public:
    explicit basic_timer_wheel(std::chrono::duration<uint64_t>                          sd,
                         std::size_t                                              ns,
                         const std::chrono::time_point<std::chrono::steady_clock> st) noexcept
        : c_idx_(0)
        , slot_duration_(sd)
        , one_loop_duration_(slot_duration_ * ns)
        , start_time_(st)
        , slots_(ns)
    {
    }

    void check_expiration(std::chrono::time_point<std::chrono::steady_clock> now)
    {
        const auto elapsed         = now - start_time_;
        auto       nSlotsTraversed = elapsed / slot_duration_;
        start_time_ += (nSlotsTraversed * slot_duration_);

        while (nSlotsTraversed--) {
            for (auto it = slots_[c_idx_].begin(); it != slots_[c_idx_].end();) {
                auto nIt = it;
                ++nIt;
                if ((*it).n_loops_ == 0) {
                    (*it).expire();
                    (*it).unlink();
                }
                else {
                    --(*it).n_loops_;
                }
                it = nIt;
            }
            c_idx_ = (c_idx_ + 1) % slots_.size();
        }
    }

    void publish(basic_expirable<MUTEX_LOCK>*                                   e,
                 const std::chrono::time_point<std::chrono::steady_clock> expirationTime) noexcept
    {
        const auto timeDiff = expirationTime - start_time_;
        const auto nSlots   = timeDiff / slot_duration_;
        const auto pIdx     = (c_idx_ + nSlots) % slots_.size();

        e->n_loops_ = timeDiff / one_loop_duration_;

        slots_[pIdx].push_back(e);
    }

private:
    size_t c_idx_;

    const std::chrono::duration<uint64_t>                 slot_duration_;
    const std::chrono::duration<uint64_t>                 one_loop_duration_;
    std::chrono::time_point<std::chrono::steady_clock>    start_time_;
    std::vector<ilist<basic_expirable<MUTEX_LOCK>, MUTEX_LOCK>> slots_;
};
} // namespace br

#endif // BR_TIMER_WHEEL_H_
