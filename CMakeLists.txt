cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(BUILD_LIBCLANGUML OFF CACHE BOOL "Build libClangUML" FORCE)
set(BUILD_CLANGUML OFF CACHE BOOL "Build ClangUML" FORCE)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION_FULL ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
add_definitions(-DCLANGUML_VERSION_MAJOR=${VERSION_MAJOR}
                -DCLANGUML_VERSION_MINOR=${VERSION_MINOR}
                -DCLANGUML_VERSION_PATCH=${VERSION_PATCH})
project(ClangUML)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(LibClang REQUIRED)
include_directories(
	${CLANG_INCLUDEDIR}
)
link_libraries(
        ${CLANG_LIBS}
)

add_subdirectory(libclanguml)
add_executable(ClangUML src/main.c)
target_link_libraries(ClangUML LINK_PUBLIC LibClangUML)
