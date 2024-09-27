set(OPENCV_HEADER_SEARCH_PATHS)
set(OPENCV_LINK_LIBRARIES)

set(OPENCV_VERSION "3.2.0")

if(${ISX_ARCH_ARM})
    set(OPENCV_DIR "${THIRD_PARTY_DIR}/OpenCV/${OPENCV_VERSION}.cuda/linux-arm")
else()
    set(OPENCV_DIR "${THIRD_PARTY_DIR}/OpenCV/${OPENCV_VERSION}.no_mkl")
    if(${ISX_OS_MACOS})
        string(APPEND OPENCV_DIR "/osx")
    elseif(${ISX_OS_LINUX})
        string(APPEND OPENCV_DIR "/linux")
    elseif(${ISX_OS_WIN32})
        string(APPEND OPENCV_DIR "/win")
    endif()
endif()

string(APPEND OPENCV_DIR "/${CMAKE_BUILD_TYPE}")

if(${ISX_OS_WIN32})
    # TODO : need to force this variable for windows, but may need
    # to check/rebuild OpenCV to correct this
    set(OpenCV_STATIC ON)
    list(APPEND CMAKE_PREFIX_PATH ${OPENCV_DIR})
else()
    list(APPEND CMAKE_PREFIX_PATH ${OPENCV_DIR}/share/OpenCV)
endif()

if(${ISX_ARCH_ARM})
    find_package(OpenCV ${OPENCV_VERSION} REQUIRED core imgproc video cudawarping)
else()
    find_package(OpenCV ${OPENCV_VERSION} REQUIRED core imgproc video features2d)
endif()

list(APPEND OPENCV_HEADER_SEARCH_PATHS ${OpenCV_INCLUDE_DIRS})
list(APPEND OPENCV_LINK_LIBRARIES ${OpenCV_LIBS})

# TODO : check if this is needed after using CMake for OpenCV
if(${ISX_OS_LINUX})
    list(APPEND OPENCV_LINK_LIBRARIES libpthread.so)
endif()
