# BR - C++ utilities

Work in progress.

#### br::ilist

Intrusive list that can be locked to be thread-safe.

#### br::timer_wheel

A timer wheel (or time wheel) which uses intrusive lists for each slot.

#### br::spinlock

A spinlock which uses the thread_id as its locking atomic.

#### br::arch_info

Multiplatform (Linux/Win32) information about the NUMA nodes in the system. Set CPU affinity for a given thread.
