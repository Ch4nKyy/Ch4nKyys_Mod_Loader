cmake_minimum_required(VERSION 3.21)

project(
    Ch4nKyysModLoader
    VERSION 1.0.0
    LANGUAGES CXX RC
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

file(MAKE_DIRECTORY "${GENERATED_DIR}")

set(VERSION_RC
    "${GENERATED_DIR}/version.rc"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in"
    "${VERSION_RC}"
    @ONLY
)

file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_library(
    Ch4nKyysModLoader
    SHARED

    ${SOURCES}
    ${HEADERS}

    version.def
    "${VERSION_RC}"
)

set_target_properties(
    Ch4nKyysModLoader
    PROPERTIES
    OUTPUT_NAME "version"
    PREFIX ""
)

target_compile_definitions(
    Ch4nKyysModLoader
    PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

if(MSVC)
    set_property(
        TARGET Ch4nKyysModLoader
        PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_compile_options(
        Ch4nKyysModLoader
        PRIVATE
        /W4
        /permissive-
    )
endif()

if(DEFINED ENV{BG3PATH} AND NOT "$ENV{BG3PATH}" STREQUAL "")
    add_custom_command(
        TARGET Ch4nKyysModLoader
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Ch4nKyysModLoader>"
        "$ENV{BG3PATH}/bin/version.dll"
    )
endif()

if(DEFINED ENV{BG3PATH2} AND NOT "$ENV{BG3PATH2}" STREQUAL "")
    add_custom_command(
        TARGET Ch4nKyysModLoader
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Ch4nKyysModLoader>"
        "$ENV{BG3PATH2}/bin/version.dll"
    )
endif()
