cmake_minimum_required(VERSION 3.5)
include(${CMAKE_CURRENT_LIST_DIR}/../../tools/cmake/configure.cmake)

file(GLOB SRCS ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB HDRS ${CMAKE_CURRENT_LIST_DIR}/api/*.h)

set(TARGET_NAME "core")

add_library(${TARGET_NAME} STATIC ${SRCS} ${HDRS})


target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/api)
target_include_directories(${TARGET_NAME} PRIVATE ${QT_HEADER_SEARCH_PATHS} ${HDF5_HEADER_SEARCH_PATHS})
set_target_properties(${TARGET_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../../lib"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../../lib"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../../lib"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../../lib"
)

include(${CMAKE_CURRENT_LIST_DIR}/../../tools/cmake/target_properties.cmake)

#[[
get_property(inc_dirs TARGET ${TARGET_NAME} PROPERTY INCLUDE_DIRECTORIES)
message("inc_dirs = ${inc_dirs}")

get_property(link_libs TARGET ${TARGET_NAME} PROPERTY LINK_LIBRARIES)
message("link_libs = ${link_libs}")
]]