add_library(awdl "")

target_sources(awdl PRIVATE
        log.c
        log.h
        state.c
        state.h
        election.c
        election.h
        sync.c
        sync.h
        channel.c
        channel.h
        schedule.c
        schedule.h
        tx.c
        tx.h
        rx.c
        rx.h
        frame.c
        frame.h
        crc32.c
        crc32.h
        wire.c
        wire.h
        peers.c
        peers.h
        version.c
        version.h
        hashmap.c
        hashmap.h
        siphash24.c
        siphash24.h
        circular_buffer.c
        circular_buffer.h
)

target_include_directories(awdl PRIVATE ${CMAKE_SOURCE_DIR}/radiotap)

target_link_libraries(awdl radiotap)
