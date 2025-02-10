include_directories(include)

add_library(br
            STATIC
            ilist.cc
            timer_wheel.cc
            spinlock.cc
            arch_info.cc
)
target_include_directories(br PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
