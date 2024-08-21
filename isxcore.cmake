# project(IdpsExample)
cmake_minimum_required(VERSION 3.5)

# define third-party dependencies of isx core API
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)

# Qt5Core
list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_DIR}/Qt/5.8/linux/lib/cmake/Qt5Core)
find_package(Qt5Core 5.8 REQUIRED)

# OpenCV
list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_DIR}/OpenCV/3.2.0.no_mkl/linux/Release/share/OpenCV)
# set(OpenCV_STATIC ON)
find_package(OpenCV 3.2.0 REQUIRED core imgproc video features2d)

# Tiff
set(TIFF_DIR ${THIRD_PARTY_DIR}/libtiff/4.0.8.isx/linux/Release/)
set(TIFF_INCLUDE_DIR ${TIFF_DIR}/include)
set(TIFF_LIB_DIR ${TIFF_DIR}/lib)
set(TIFF_LIBS ${TIFF_LIB_DIR}64/libtiff.a)

# HDF5
set(HDF5_DIR ${THIRD_PARTY_DIR}/hdf5/1.10/linux)
set(HDF5_INCLUDE_DIR ${HDF5_DIR}/include)
set(HDF5_LIB_DIR ${HDF5_DIR}/lib-fPIC)
set(HDF5_LIBS ${HDF5_LIB_DIR}/libhdf5_cpp.a)
list(APPEND HDF5_LIBS ${HDF5_LIB_DIR}/libhdf5.a)

# ffmpeg
set(FFMPEG_DIR ${THIRD_PARTY_DIR}/ffmpeg/3.1.4/linux)
set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)
set(FFMPEG_LIBS ${FFMPEG_LIB_DIR}/libavformat.so.57)
list(APPEND FFMPEG_LIBS ${FFMPEG_LIB_DIR}/libavcodec.so.57)
list(APPEND FFMPEG_LIBS ${FFMPEG_LIB_DIR}/libavutil.so.55)
# list(APPEND FFMPEG_LIBS ${FFMPEG_LIB_DIR}/libswresample.so.2)

# Boost
set(BOOST_DIR ${THIRD_PARTY_DIR}/boost/1.72.0/linux)
set(BOOST_INCLUDE_DIR ${BOOST_DIR}/include)
set(BOOST_LIB_DIR ${BOOST_DIR}/lib)
set(BOOST_LIBS ${BOOST_LIB_DIR}/libboost_serialization.a)
list(APPEND BOOST_LIBS ${BOOST_LIB_DIR}/libboost_program_options.a)

# json
set(JSON_INCLUDE_DIR ${THIRD_PARTY_DIR}/json_modern_C++/2.0.1_isx)

# define module paths
set(MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/modules)
set(MODULES_LIB_DIR ${MODULES_DIR}/lib)
set(MODULES_THIRD_PARTY_DIR ${MODULES_DIR}/third_party)

# define isx core paths
set(ISX_CORE_INCLUDE_DIRS
    ${MODULES_DIR}/include/isxcore
    ${Qt5Core_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIR}
    ${HDF5_INCLUDE_DIR}
    ${FFMPEG_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${JSON_INCLUDE_DIR}
)

set(ISX_CORE_LIBS
    ${MODULES_LIB_DIR}/libisxcore.a
    ${Qt5Core_LIBRARIES}
    ${OpenCV_LIBS}
    ${TIFF_LIBS}
    ${HDF5_LIBS}
    ${FFMPEG_LIBS}
    ${BOOST_LIBS}
    ZLIB::ZLIB
    ${CMAKE_DL_LIBS} # needed for hdf5
)

set(ISX_CORE_THIRD_PARTY_DIR ${MODULES_THIRD_PARTY_DIR}/isxcore)
