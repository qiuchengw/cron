include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")

link_directories("${LIBRARY_OUTPUT_PATH}")

add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DDEBUG)

