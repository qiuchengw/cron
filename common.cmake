include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")

link_directories("${LIBRARY_OUTPUT_PATH}")

add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DDEBUG)
add_definitions(-DNOMINMAX)

IF(MSVC)
#     ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/src/xxx")
ELSE()
    message(STATUS " ---> for NOT MSVC, we use SS_ANSI to make stdstring compile^")
    add_definitions(-DSS_ANSI)
ENDIF()

