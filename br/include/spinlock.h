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

#ifndef BR_SPINLOCK_H_
#define BR_SPINLOCK_H_


#include <atomic>
#include <thread>
#include <cassert>

namespace br {
class spinlock {
public:
    spinlock() noexcept             = default;
    spinlock(spinlock&)             = delete;
    spinlock(spinlock&&)            = delete;
    spinlock& operator=(spinlock&)  = delete;
    spinlock& operator=(spinlock&&) = delete;

    void lock() noexcept
    {
        auto expected = static_cast<std::thread::id>(0);
        while (!l_.compare_exchange_weak(expected,
                                         std::this_thread::get_id(),
                                         std::memory_order_release,
                                         std::memory_order_acquire)) {
            expected = static_cast<std::thread::id>(0);
#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
            __asm ("pause");
#endif
         }
    }

    void unlock() noexcept
    {
        auto       expected    = std::this_thread::get_id();
        const bool same_thread = l_.compare_exchange_strong(expected,
                                                            static_cast<std::thread::id>(0),
                                                            std::memory_order_release,
                                                            std::memory_order_acquire);
        assert(
            same_thread &&
            "This thread is trying to unlock a non-locked spinlock or locked by another thread");
    }


    [[nodiscard]] bool try_lock() noexcept
    {
        auto expected = static_cast<std::thread::id>(0);
        return !l_.compare_exchange_strong(expected,
                                           std::this_thread::get_id(),
                                           std::memory_order_release,
                                           std::memory_order_relaxed);
    }

private:
    std::atomic<std::thread::id> l_{static_cast<std::thread::id>(0)};

    // Please note:
    static_assert(std::atomic<std::thread::id>::is_always_lock_free,
                  "Thread::id must be a real basic atomic to use this implementation");
};
} // namespace br

#endif /* BR_SPINLOCK_H_ */
