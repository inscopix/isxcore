cmake_minimum_required(VERSION 3.5)
set(ISX_CMAKE_TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR}/tools/cmake)
include(${ISX_CMAKE_TOOLS_DIR}/configure.cmake)

include(${ISX_CMAKE_TOOLS_DIR}/core.cmake)

include(${ISX_CMAKE_TOOLS_DIR}/mostest.cmake)
