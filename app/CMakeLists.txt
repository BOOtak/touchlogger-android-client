cmake_minimum_required(VERSION 3.4.1)

set(ASSETS_PATH "${CMAKE_CURRENT_LIST_DIR}/src/main/assets")
set(EXECUTABLE_OUTPUT_PATH "${ASSETS_PATH}/${ANDROID_ABI}")

# TODO: make it depend on actual debug flag
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG=1" )

add_library(dirtycopy SHARED
    src/main/cpp/native-lib.cpp
    src/main/cpp/dirty_copy.c
    )

target_link_libraries(dirtycopy
    PUBLIC log
    )

add_executable(run-as
    src/main/cpp/run-as.c
    )

target_link_libraries(run-as
    PUBLIC log
    )
