cmake_minimum_required(VERSION 3.4.1)

set(ASSETS_PATH "${CMAKE_CURRENT_LIST_DIR}/src/main/assets")
set(EXECUTABLE_OUTPUT_PATH "${ASSETS_PATH}/${ANDROID_ABI}")

# TODO: make it depend on actual debug flag
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG=1 -DDEBUG_POINTER_ASSIGNMENT=1 -DDEBUG_RAW_EVENTS=1 -DDEBUG_POINTERS=1 -fvisibility=hidden -s -Os -fno-stack-protector -fmerge-all-constants -fno-unwind-tables -fno-asynchronous-unwind-tables" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG=1 -DDEBUG_POINTER_ASSIGNMENT=1 -DDEBUG_RAW_EVENTS=1 -DDEBUG_POINTERS=1 -fvisibility=hidden -s -Os" )

set(SRC_DIR src/main/cpp)

set(DIRTY_SOURCES
    "${SRC_DIR}/native-lib.cpp"
    "${SRC_DIR}/dirty/common/common.c"
    "${SRC_DIR}/dirty/file_utils/file_utils.c"
    "${SRC_DIR}/dirty/file_utils/dirty_copy.c"
    "${SRC_DIR}/dirty/lib_utils/inject.c"
    "${SRC_DIR}/dirty/lib_utils/elf_parser.c"
    )

set(PAYLOAD_SOURCES
    "${SRC_DIR}/payload.c"
    )

add_library(dirtycopy SHARED
    ${DIRTY_SOURCES}
    )

target_link_libraries(dirtycopy
    PUBLIC log
    )

add_executable(dirty
    ${DIRTY_SOURCES}
    "${SRC_DIR}/dirty/main.c"
    )

target_link_libraries(dirty
    PUBLIC log
    )

add_executable(exec_payload
    ${SRC_DIR}/exec_payload.c
    )

target_link_libraries(exec_payload
    PUBLIC log
    )

add_executable(payload
    ${PAYLOAD_SOURCES}
    )

target_link_libraries(payload
    PUBLIC log
    )

add_custom_command(TARGET payload
    POST_BUILD
    COMMAND ${CMAKE_STRIP} -S --strip-unneeded
                              --remove-section=.note.gnu.gold-version
                              --remove-section=.comment
                              ${EXECUTABLE_OUTPUT_PATH}/payload
    COMMAND ${CMAKE_STRIP} -S --remove-section=.note
                              --remove-section=.note.gnu.build-id
                              --remove-section=.note.ABI-tag
                              ${EXECUTABLE_OUTPUT_PATH}/payload
    COMMAND ${CMAKE_STRIP} -S --remove-section=.got.plt
                              --remove-section=.eh_frame
                              --remove-section=.eh_frame_ptr
                              --remove-section=.eh_frame_hdr
                              ${EXECUTABLE_OUTPUT_PATH}/payload
    )

add_library(shared_payload SHARED
    ${PAYLOAD_SOURCES}
    )

target_link_libraries(shared_payload
    PUBLIC log
    )

set_target_properties(shared_payload PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${ASSETS_PATH}/${ANDROID_ABI}"
    )

add_executable(test_input
    ${SRC_DIR}/input_device/input_device_utils.cpp
    ${SRC_DIR}/input_device/runner.cpp
    ${SRC_DIR}/input_device/mappers/MultitouchInputMapper.cpp
    ${SRC_DIR}/input_device/mappers/TouchInputMapper.cpp
    ${SRC_DIR}/input_device/accumulators/MultiTouchMotionAccumulator.cpp
    ${SRC_DIR}/input_device/utils/CookedPointData.cpp
    ${SRC_DIR}/input_device/utils/InputDevice.cpp
    ${SRC_DIR}/input_device/utils/PointerCoords.cpp
    ${SRC_DIR}/input_device/utils/RawPointerAxes.cpp
    ${SRC_DIR}/input_device/utils/RawPointerData.cpp
    )

target_link_libraries(test_input
    PUBLIC log
    )
