cmake_minimum_required(VERSION 2.8)

project(LibClangUML)
find_package(LibClang REQUIRED)
include_directories(
	${CLANG_INCLUDEDIR}
)

add_library(LibClangUML clanguml.c)
target_include_directories(LibClangUML PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
