set(TARGET_NAME_MOSTEST "mostest")
set(MOSTEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../test/src)

file(GLOB MOSTEST_SRCS ${MOSTEST_SRC_DIR}/*.cpp)
file(GLOB MOSTEST_HDRS ${MOSTEST_SRC_DIR}/*.h)
file(GLOB CORE_TEST_SRCS ${CORE_TEST_DIR}/*.cpp)
file(GLOB CORE_TEST_HDRS ${CORE_TEST_DIR}/*.h)
# file(GLOB ALGO_TEST_SRCS ${ALGO_TEST_DIR}/*.cpp)
# file(GLOB ALGO_TEST_HDRS ${ALGO_TEST_DIR}/*.h)
# file(GLOB PLAYER_TEST_SRCS ${PLAYER_TEST_DIR}/*.cpp)
# file(GLOB PLAYER_TEST_HDRS ${PLAYER_TEST_DIR}/*.h)
# file(GLOB DRM_TEST_SRCS ${DRM_TEST_DIR}/*.cpp)
# file(GLOB DRM_TEST_HDRS ${DRM_TEST_DIR}/*.h)

if(${ISX_ARCH_ARM})
    add_executable(${TARGET_NAME_MOSTEST}
        ${MOSTEST_SRCS} ${MOSTEST_HDRS}
        ${CORE_TEST_SRCS} ${CORE_TEST_HDRS}
        # ${ALGO_TEST_SRCS} ${ALGO_TEST_HDRS}
    )

    target_include_directories(${TARGET_NAME_MOSTEST} PRIVATE
        ${MOSTEST_SRC_DIR}
        ${CATCH_HEADER_SEARCH_PATHS}
        ${CORE_SRC_DIR}             # to allow testing internal-only classes
        # ${ALGO_SRC_DIR}
        ${HDF5_HEADER_SEARCH_PATHS}
        ${JSON_HEADER_SEARCH_PATHS}
        ${OPENCV_HEADER_SEARCH_PATHS}
        # ${ARMADILLO_HEADER_SEARCH_PATHS}
        ${QT_CORE_HEADER_SEARCH_PATHS}
        ${LIBTIFF_HEADER_SEARCH_PATHS}
        # ${SCS_HEADER_SEARCH_PATHS}
        # ${CVXCORE_HEADER_SEARCH_PATHS}
        # ${EIGEN_HEADER_SEARCH_PATHS}
        # ${CERES_HEADER_SEARCH_PATHS}
        ${BOOST_HEADER_SEARCH_PATHS})

    target_link_libraries(${TARGET_NAME_MOSTEST} PRIVATE 
        # ${TARGET_NAME_ALGO}         # ALGO depends on CORE, gcc linker wants link dependencies specified in order
        ${TARGET_NAME_CORE})

else()
    add_executable(${TARGET_NAME_MOSTEST}
        ${MOSTEST_SRCS} ${MOSTEST_HDRS}
        ${CORE_TEST_SRCS} ${CORE_TEST_HDRS}
        # ${ALGO_TEST_SRCS} ${ALGO_TEST_HDRS}
        # ${PLAYER_TEST_SRCS} ${PLAYER_TEST_HDRS}
        # ${DRM_TEST_SRCS} ${DRM_TEST_HDRS}
    )

    target_include_directories(${TARGET_NAME_MOSTEST} PRIVATE
        ${MOSTEST_SRC_DIR}
        ${CATCH_HEADER_SEARCH_PATHS}
        ${CORE_SRC_DIR}             # to allow testing internal-only classes
        # ${ALGO_SRC_DIR}
        # ${PLAYER_SRC_DIR}
        ${HDF5_HEADER_SEARCH_PATHS}
        ${JSON_HEADER_SEARCH_PATHS}
        ${OPENCV_HEADER_SEARCH_PATHS}
        # ${ARMADILLO_HEADER_SEARCH_PATHS}
        ${QT_CORE_HEADER_SEARCH_PATHS}
        ${LIBTIFF_HEADER_SEARCH_PATHS}
        # ${SCS_HEADER_SEARCH_PATHS}
        # ${CVXCORE_HEADER_SEARCH_PATHS}
        # ${EIGEN_HEADER_SEARCH_PATHS}
        # ${CERES_HEADER_SEARCH_PATHS}
        # ${BOOST_HEADER_SEARCH_PATHS}
    )

    target_link_libraries(${TARGET_NAME_MOSTEST} PRIVATE
        # ${TARGET_NAME_ALGO}         # ALGO depends on CORE, gcc linker wants link dependencies specified in order
        # ${TARGET_NAME_PLAYER}
        # ${TARGET_NAME_DRM}
        ${TARGET_NAME_CORE})

    # We copy the FlexNet shared libraries because it seems like they must be
    # dynamically loaded, in which case its most convenient to have them next to
    # mostest binary (e.g. for VS and XCode).
    # Same with the dynamic libraries used by SCS.
    # installFlexeraSharedLibs(${TARGET_NAME_MOSTEST} "${CMAKE_BINARY_DIR}/../bin")
    # installScsSharedLibs(${TARGET_NAME_MOSTEST} "${CMAKE_BINARY_DIR}/../bin")
endif()

set_target_properties(${TARGET_NAME_MOSTEST} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin"
)

setCommonCxxOptionsForTarget(${TARGET_NAME_MOSTEST})
setOsDefinesForTarget(${TARGET_NAME_MOSTEST})
target_compile_definitions(${TARGET_NAME_MOSTEST} PRIVATE
    # ${ARMADILLO_DEFINITIONS}
    # ${CERES_DEFINITIONS}
    ISX_WITH_CUDA=${ISX_WITH_CUDA}
    ISX_DONT_LOG_EXCEPTIONS
)
disableVisualStudioWarnings(${TARGET_NAME_MOSTEST})
