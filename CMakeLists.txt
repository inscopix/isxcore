cmake_minimum_required(VERSION 3.5)
include(${CMAKE_CURRENT_LIST_DIR}/../../tools/cmake/configure.cmake)

file(GLOB SRCS ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB HDRS ${CMAKE_CURRENT_LIST_DIR}/include/*.h)

add_library(core STATIC ${SRCS} ${HDRS})

#target_include_directories(core ${CMAKE_CURRENT_LIST_DIR}/include ${QT_HEADER_SEARCH_PATHS})
target_include_directories(core PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(core PRIVATE ${QT_HEADER_SEARCH_PATHS})
target_link_libraries(core ${QT_LINK_LIBRARIES})
set_target_properties(core PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../../lib"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../../lib"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../../lib"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../../lib"
)

#[[
get_property(inc_dirs TARGET core PROPERTY INCLUDE_DIRECTORIES)
message("inc_dirs = ${inc_dirs}")

get_property(link_libs TARGET core PROPERTY LINK_LIBRARIES)
message("link_libs = ${link_libs}")
]]