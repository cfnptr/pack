cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

project(pack 
	VERSION 0.1.0
	DESCRIPTION "Simple data packing library"
	HOMEPAGE_URL "https://github.com/cfnptr/pack"
	LANGUAGES C)

set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/zstd/build/cmake)

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if (IS_BIG_ENDIAN)
	set(PACK_LITTLE_ENDIAN 0)
else ()
	set(PACK_LITTLE_ENDIAN 1)
endif ()

configure_file(
	cmake/defines.h.in
	pack/defines.h)

add_library(pack STATIC
	source/pack_writer.c
	source/pack_reader.c)
target_link_libraries(pack PUBLIC
	libzstd_static)
target_include_directories(pack PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/libraries/zstd/lib)
