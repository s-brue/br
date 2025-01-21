cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
        )

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

include_directories(${br_SOURCE_DIR}/src/include
                    ${br_SOURCE_DIR}/src/include
                    )

add_executable(brTS
               ilist_ts.cc
               timer_wheel_ts.cc
               spinlock_ts.cc
)

target_link_libraries(brTS GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(brTS)
