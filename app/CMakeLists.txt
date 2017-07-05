cmake_minimum_required(VERSION 3.4.1)

set(ASSETS_PATH "${CMAKE_CURRENT_LIST_DIR}/src/main/assets")
set(EXECUTABLE_OUTPUT_PATH "${ASSETS_PATH}/${ANDROID_ABI}")

# TODO: make it depend on actual debug flag
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG=1 -fvisibility=hidden -s -Os -fno-stack-protector -fmerge-all-constants -fno-unwind-tables -fno-asynchronous-unwind-tables" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG=1 -fvisibility=hidden -s -Os" )

set(SRC_DIR src/main/cpp)

add_library(dirtycopy SHARED
    ${SRC_DIR}/native-lib.cpp
    ${SRC_DIR}/dirty/common/common.c
    ${SRC_DIR}/dirty/file_utils/file_utils.c
    ${SRC_DIR}/dirty/file_utils/dirty_copy.c
    ${SRC_DIR}/dirty/lib_utils/inject.c
    ${SRC_DIR}/dirty/lib_utils/elf_parser.c
    )

set_target_properties(dirtycopy PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${ASSETS_PATH}/${ANDROID_ABI}"
    )

target_link_libraries(dirtycopy
    PUBLIC log
    )

add_library(shared_payload SHARED
    ${SRC_DIR}/dirty/file_utils/file_utils.c
    ${SRC_DIR}/shared_payload.c
    )

set_target_properties(shared_payload PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${ASSETS_PATH}/${ANDROID_ABI}"
    )

target_link_libraries(shared_payload
    PUBLIC log
    )

add_executable(exec_payload
    ${SRC_DIR}/exec_payload.c
    )

target_link_libraries(exec_payload
    PUBLIC log
    )