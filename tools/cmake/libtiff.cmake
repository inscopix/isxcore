set(LIBTIFF_HEADER_SEARCH_PATHS)
set(LIBTIFF_LINK_LIBRARIES)
set(LIBTIFF_DIR ${THIRD_PARTY_DIR}/libtiff/4.0.8.isx)
if(${ISX_OS_MACOS})
    string(APPEND LIBTIFF_DIR "/osx")
elseif(${ISX_OS_LINUX})
    string(APPEND LIBTIFF_DIR "/linux")
elseif(${ISX_OS_WIN32})
    string(APPEND LIBTIFF_DIR "/win")
endif()

if(${ISX_ARCH_ARM})
    string(APPEND LIBTIFF_DIR "-arm")
endif()

string(APPEND LIBTIFF_DIR "/${CMAKE_BUILD_TYPE}")

set(a ${LIBTIFF_DIR})
string(APPEND a "/include")
list(APPEND LIBTIFF_HEADER_SEARCH_PATHS ${a})

string(APPEND LIBTIFF_DIR "/lib")

if(${ISX_OS_MACOS})
    list(APPEND LIBTIFF_LINK_LIBRARIES "${LIBTIFF_DIR}/libtiff.a")

elseif(${ISX_OS_LINUX})
    if(${ISX_ARCH_ARM})
        list(APPEND LIBTIFF_LINK_LIBRARIES "${LIBTIFF_DIR}/libtiff.a")
    else()
        list(APPEND LIBTIFF_LINK_LIBRARIES "${LIBTIFF_DIR}64/libtiff.a")
    endif()

elseif(${ISX_OS_WIN32})
    if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
        list(APPEND LIBTIFF_LINK_LIBRARIES "${LIBTIFF_DIR}/tiffd.lib")
    else()
        list(APPEND LIBTIFF_LINK_LIBRARIES "${LIBTIFF_DIR}/tiff.lib")
    endif()
endif()

