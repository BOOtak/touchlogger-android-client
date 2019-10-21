cmake_minimum_required(VERSION 3.4.1)

set(ASSETS_PATH "${CMAKE_CURRENT_LIST_DIR}/src/main/assets")
set(EXECUTABLE_OUTPUT_PATH "${ASSETS_PATH}/${ANDROID_ABI}")

# TODO: make it depend on actual debug flag
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG=1 -DDEBUG_POINTER_ASSIGNMENT=0 -fvisibility=hidden -s -Os -fno-stack-protector -fmerge-all-constants -fno-unwind-tables -fno-asynchronous-unwind-tables" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG=1 -DDEBUG_POINTER_ASSIGNMENT=0 -fvisibility=hidden -s -Os" )

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp")

set(DIRTY_SOURCES
    "${SRC_DIR}/native-lib.cpp"
    "${SRC_DIR}/utils/net_utils.c"
    "${SRC_DIR}/dirty/file_utils/file_utils.c"
    )

add_library(input_device STATIC
    "${SRC_DIR}/ControlReader.cpp"
    "${SRC_DIR}/utils/net_utils.c"
    "${SRC_DIR}/input_device/EventFileWriter.cpp"
    "${SRC_DIR}/input_device/TestEventFileWriter.cpp"
    "${SRC_DIR}/input_device/InputReader.cpp"
    "${SRC_DIR}/input_device/accumulators/MultiTouchMotionAccumulator.cpp"
    "${SRC_DIR}/input_device/mappers/MultitouchInputMapper.cpp"
    "${SRC_DIR}/input_device/mappers/TouchInputMapper.cpp"
    "${SRC_DIR}/input_device/utils/CookedPointData.cpp"
    "${SRC_DIR}/input_device/utils/InputDevice.cpp"
    "${SRC_DIR}/input_device/utils/PointerCoords.cpp"
    "${SRC_DIR}/input_device/utils/RawPointerAxes.cpp"
    "${SRC_DIR}/input_device/utils/RawPointerData.cpp"
    "${SRC_DIR}/input_device/utils/Utils.cpp"
    )

target_include_directories(input_device PUBLIC
    $<BUILD_INTERFACE:${SRC_DIR}/input_device>
    $<INSTALL_INTERFACE:input_device>
)

add_library(dirtycopy SHARED
    ${DIRTY_SOURCES}
    )

target_link_libraries(dirtycopy
    PUBLIC log
    )

add_executable(exec_payload
    ${SRC_DIR}/exec_payload.cpp
    ${SRC_DIR}/utils/Reanimator.cpp
    )

target_link_libraries(exec_payload
    PUBLIC log input_device
    )
