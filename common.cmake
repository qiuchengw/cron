include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")

link_directories("${LIBRARY_OUTPUT_PATH}")

add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DDEBUG)
add_definitions(-DNOMINMAX)

IF (WIN32)
    IF(MSVC)
        MESSAGE(STATUS " ---> WIN32 && msvc compile^")
    ELSE()
        MESSAGE(STATUS " ---> for UNIX, we de SS_ANSI to make stdstring compile^")
        add_definitions(-DSS_ANSI)
    ENDIF()
ENDIF ()


