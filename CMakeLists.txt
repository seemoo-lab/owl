cmake_minimum_required(VERSION 3.6)

project(owl)

if (CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options("-Wall" "-Wextra")
endif ()

add_library(radiotap radiotap/radiotap.c)
target_compile_definitions(radiotap PRIVATE -D_BSD_SOURCE -DRADIOTAP_SUPPORT_OVERRIDES)

add_subdirectory(src)

add_subdirectory(daemon)

option(INSTALL_GTEST "" OFF)
add_subdirectory(googletest)

add_subdirectory(tests)
