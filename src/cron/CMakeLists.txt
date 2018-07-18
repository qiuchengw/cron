cmake_minimum_required(VERSION 2.8)

project(cron)

message(STATUS " building cron ...")
message(STATUS " source dir: " ${PROJECT_SOURCE_DIR})
message(STATUS " top source dir: " ${TOP_SOURCE_PATH})

include(${TOP_SOURCE_PATH}/common.cmake)

file(GLOB SRC_FILES  ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.h)
# file(GLOB CONF_FILES  ${PROJECT_SOURCE_DIR}/../*.cpp ${PROJECT_SOURCE_DIR}/../*.h)
# list(REMOVE_ITEM SRC_FILES  ${PROJECT_SOURCE_DIR}/dog.cpp ${PROJECT_SOURCE_DIR}/dog.h)

set(VERSION 1.0.0)
add_library(cron ${SRC_FILES})
target_link_libraries(cron)
