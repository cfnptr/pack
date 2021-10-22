cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

if (TARGET pack)
	return()
endif ()

project(pack 
	VERSION 1.0.1
	DESCRIPTION "Simple data packing library"
	HOMEPAGE_URL "https://github.com/cfnptr/pack"
	LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

option(PACK_BUILD_SHARED "Build PACK shared library" ON)
option(PACK_BUILD_UTILITIES "Build PACK utility programs" ON)

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

set(PACK_SOURCES
	source/common.c
	source/reader.c
	source/writer.c)

add_library(pack-static STATIC
	${PACK_SOURCES})
set_target_properties(pack-static PROPERTIES
	OUTPUT_NAME "pack")
target_link_libraries(pack-static PUBLIC
	libzstd_static)
target_include_directories(pack-static PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/libraries/zstd/lib)

if (PACK_BUILD_SHARED)
	add_library(pack-shared SHARED
		${PACK_SOURCES})
	set_target_properties(pack-shared PROPERTIES
		OUTPUT_NAME "pack"
		WINDOWS_EXPORT_ALL_SYMBOLS ON)
	target_link_libraries(pack-shared PUBLIC
		libzstd_static)
	target_include_directories(pack-shared PUBLIC
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/libraries/zstd/lib)
endif ()

if (PACK_BUILD_UTILITIES)
	add_executable(packer
		utilities/packer.c)
	target_link_libraries(packer PRIVATE
		pack-static)
	target_include_directories(packer PRIVATE
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include)

	add_executable(unpacker
		utilities/unpacker.c)
	target_link_libraries(unpacker PRIVATE
		pack-static)
	target_include_directories(unpacker PRIVATE
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include)

	add_executable(pack-info
		utilities/pack_info.c)
	target_link_libraries(pack-info PRIVATE
		pack-static)
	target_include_directories(pack-info PRIVATE
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include)
endif ()
