cmake_minimum_required(VERSION 2.8)

project(shell)

message(STATUS " building shell ...")
message(STATUS " source dir: " ${PROJECT_SOURCE_DIR})
message(STATUS " top source dir: " ${TOP_SOURCE_PATH})

include(${TOP_SOURCE_PATH}/common.cmake)

file(GLOB SRC_FILES  ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.h)

set(VERSION 1.0.0)
add_executable(shell ${SRC_FILES})
target_link_libraries(shell cron)
