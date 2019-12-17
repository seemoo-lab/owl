set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(tests "")

target_sources(tests PRIVATE
        test_wire.cpp
        test_awdl_sync.cpp
        test_awdl_peers.cpp
        test_awdl_election.cpp
)

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(tests gtest gtest_main)
target_link_libraries(tests awdl)
