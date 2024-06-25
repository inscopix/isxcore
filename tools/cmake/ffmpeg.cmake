set (FFMPEG_HEADER_SEARCH_PATHS)
set (FFMPEG_LINK_LIBRARIES)

set(FFMPEG_DIR ${THIRD_PARTY_DIR}/ffmpeg/3.1.4)
if(${ISX_OS_MACOS})
    string(APPEND FFMPEG_DIR "/osx")
elseif(${ISX_OS_LINUX})
    string(APPEND FFMPEG_DIR "/linux")
elseif(${ISX_OS_WIN32})
    string(APPEND FFMPEG_DIR "/win")
endif()

if(${ISX_ARCH_ARM})
    string(APPEND FFMPEG_DIR "-arm")
endif()

if(${ISX_OS_MACOS})
    set(a ${FFMPEG_DIR})
    string(APPEND a "/lib/libavformat.57.dylib")
    list(APPEND FFMPEG_LINK_LIBRARIES ${a})

    set(a ${FFMPEG_DIR})
    string(APPEND a "/lib/libavcodec.57.dylib")
    list(APPEND FFMPEG_LINK_LIBRARIES ${a})

    set(a ${FFMPEG_DIR})
    string(APPEND a "/lib/libavutil.55.dylib")
    list(APPEND FFMPEG_LINK_LIBRARIES ${a})

    set(a ${FFMPEG_DIR})
    string(APPEND a "/include")
    list(APPEND FFMPEG_HEADER_SEARCH_PATHS ${a})

elseif(${ISX_OS_LINUX})

    set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/lib")

    list(APPEND FFMPEG_LINK_LIBRARIES "${FFMPEG_LIB_DIR}/libavformat.so.57")
    list(APPEND FFMPEG_LINK_LIBRARIES "${FFMPEG_LIB_DIR}/libavcodec.so.57")
    list(APPEND FFMPEG_LINK_LIBRARIES "${FFMPEG_LIB_DIR}/libavutil.so.55")

    list(APPEND FFMPEG_HEADER_SEARCH_PATHS "${FFMPEG_DIR}/include")

elseif(${ISX_OS_WIN32})
    set(a ${FFMPEG_DIR})
    string(APPEND a "/lib/avformat.lib")
    list(APPEND FFMPEG_LINK_LIBRARIES ${a})

    set(a ${FFMPEG_DIR})
    string(APPEND a "/lib/avcodec.lib")
    list(APPEND FFMPEG_LINK_LIBRARIES ${a})

    set(a ${FFMPEG_DIR})
    string(APPEND a "/lib/avutil.lib")
    list(APPEND FFMPEG_LINK_LIBRARIES ${a})

    set(a ${FFMPEG_DIR})
    string(APPEND a "/include")
    list(APPEND FFMPEG_HEADER_SEARCH_PATHS ${a})
endif()
