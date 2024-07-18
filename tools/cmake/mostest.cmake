set(TARGET_NAME_MOSTEST "mostest")
set(MOSTEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../test/src)

file(GLOB MOSTEST_SRCS ${MOSTEST_SRC_DIR}/*.cpp)
file(GLOB MOSTEST_HDRS ${MOSTEST_SRC_DIR}/*.h)
file(GLOB CORE_TEST_SRCS ${CORE_TEST_DIR}/*.cpp)
file(GLOB CORE_TEST_HDRS ${CORE_TEST_DIR}/*.h)

if(${ISX_ARCH_ARM})
    add_executable(${TARGET_NAME_MOSTEST}
        ${MOSTEST_SRCS} ${MOSTEST_HDRS}
        ${CORE_TEST_SRCS} ${CORE_TEST_HDRS}
    )

    target_include_directories(${TARGET_NAME_MOSTEST} PRIVATE
        ${MOSTEST_SRC_DIR}
        ${CATCH_HEADER_SEARCH_PATHS}
        ${CORE_SRC_DIR}             # to allow testing internal-only classes
        ${HDF5_HEADER_SEARCH_PATHS}
        ${JSON_HEADER_SEARCH_PATHS}
        ${OPENCV_HEADER_SEARCH_PATHS}
        ${QT_CORE_HEADER_SEARCH_PATHS}
        ${LIBTIFF_HEADER_SEARCH_PATHS}
        ${BOOST_HEADER_SEARCH_PATHS})

    target_link_libraries(${TARGET_NAME_MOSTEST} PRIVATE 
        ${TARGET_NAME_CORE})

else()
    add_executable(${TARGET_NAME_MOSTEST}
        ${MOSTEST_SRCS} ${MOSTEST_HDRS}
        ${CORE_TEST_SRCS} ${CORE_TEST_HDRS}
    )

    target_include_directories(${TARGET_NAME_MOSTEST} PRIVATE
        ${MOSTEST_SRC_DIR}
        ${CATCH_HEADER_SEARCH_PATHS}
        ${CORE_SRC_DIR}             # to allow testing internal-only classes
        ${HDF5_HEADER_SEARCH_PATHS}
        ${JSON_HEADER_SEARCH_PATHS}
        ${OPENCV_HEADER_SEARCH_PATHS}
        ${QT_CORE_HEADER_SEARCH_PATHS}
        ${LIBTIFF_HEADER_SEARCH_PATHS}
    )

    target_link_libraries(${TARGET_NAME_MOSTEST} PRIVATE
        ${TARGET_NAME_CORE})
endif()

set_target_properties(${TARGET_NAME_MOSTEST} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin"
)

setCommonCxxOptionsForTarget(${TARGET_NAME_MOSTEST})
setOsDefinesForTarget(${TARGET_NAME_MOSTEST})
target_compile_definitions(${TARGET_NAME_MOSTEST} PRIVATE
    ISX_DONT_LOG_EXCEPTIONS
)
disableVisualStudioWarnings(${TARGET_NAME_MOSTEST})
