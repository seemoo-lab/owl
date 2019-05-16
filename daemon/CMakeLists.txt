set(CMAKE_CXX_STANDARD 11)

include(FindPackageHandleStandardArgs)

set(SOURCES PRIVATE
        io.c
        io.h
        core.c
        core.h
        netutils.c
        netutils.h)

if (APPLE)
    list(APPEND SOURCES corewlan.m corewlan.h)
endif ()

add_executable(owl "")

target_sources(owl ${SOURCES} owl.c)

find_library(libpcap_LIBRARY NAMES pcap)
find_path(libpcap_INCLUDE pcap.h)
find_package_handle_standard_args(libpcap DEFAULT_MSG libpcap_LIBRARY libpcap_INCLUDE)
if (NOT libpcap_FOUND)
    message(FATAL_ERROR "libpcap not found")
endif ()

find_library(libev_LIBRARY NAMES ev)
find_path(libev_INCLUDE ev.h PATHS /usr/local/include)
find_package_handle_standard_args(libev DEFAULT_MSG libev_LIBRARY libev_INCLUDE)
if (NOT libev_FOUND)
    message(FATAL_ERROR "libev not found")
endif ()

if (APPLE)
    find_library(FOUNDATION Foundation)
    find_library(COREWLAN CoreWLAN)
    find_library(SYSTEMCONFIGURATION SystemConfiguration)
endif ()

target_include_directories(owl PRIVATE ${CMAKE_SOURCE_DIR}/src ${libev_INCLUDE})

target_link_libraries(owl awdl ${libpcap_LIBRARY} ${libev_LIBRARY})
if (APPLE)
    target_link_libraries(owl ${FOUNDATION} ${COREWLAN} ${SYSTEMCONFIGURATION})
else ()
    target_include_directories(owl PRIVATE /usr/include/libnl3)
    target_link_libraries(owl nl-3 nl-genl-3 nl-route-3)
endif ()

install(TARGETS owl DESTINATION bin)
