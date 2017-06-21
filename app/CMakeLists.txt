cmake_minimum_required(VERSION 3.4.1)

set(ASSETS_PATH "${CMAKE_CURRENT_LIST_DIR}/src/main/assets")
set(EXECUTABLE_OUTPUT_PATH "${ASSETS_PATH}/${ANDROID_ABI}")

# TODO: make it depend on actual debug flag
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG=1" )

add_library(dirtycopy SHARED
    src/main/cpp/libdirty/common/common.c
    src/main/cpp/libdirty/dirty_copy.c
    src/main/cpp/libdirty/dirty_inject.c
    src/main/cpp/libdirty/elf_breaker.c
    src/main/cpp/native-lib.cpp
    src/main/cpp/elf_parser.c
    )

target_link_libraries(dirtycopy
    PUBLIC log
    )

add_library(run-as SHARED
    src/main/cpp/run-as.c
    src/main/cpp/libdirty/common/common.c
    src/main/cpp/libdirty/elf_breaker.c
    )

add_executable(exec_payload
    src/main/cpp/exec_payload.c
    )

target_link_libraries(exec_payload
    PUBLIC log
    )

set_target_properties(run-as PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${ASSETS_PATH}/${ANDROID_ABI}"
    )

target_link_libraries(run-as
    PUBLIC log
    )
