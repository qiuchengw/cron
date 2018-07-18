cmake_minimum_required(VERSION 2.8)

project(test)

message(STATUS " building test ...")
message(STATUS " source dir: " ${PROJECT_SOURCE_DIR})
message(STATUS " top source dir: " ${TOP_SOURCE_PATH})

include(${TOP_SOURCE_PATH}/common.cmake)

file(GLOB SRC_FILES  ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.h)

set(VERSION 1.0.0)
add_executable(test ${SRC_FILES})
target_link_libraries(test cron)
