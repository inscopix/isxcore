function(setCommonCxxOptionsForTarget targetName)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        list(APPEND GLOBAL_CXX_OPTIONS "-Wall")
        list(APPEND GLOBAL_CXX_OPTIONS "-Wsign-compare")
        list(APPEND GLOBAL_CXX_OPTIONS "-Werror")
        list(APPEND GLOBAL_CXX_OPTIONS "-std=c++11")
        list(APPEND GLOBAL_CXX_OPTIONS "-stdlib=libc++")

        if(${CMAKE_BUILD_TYPE} MATCHES "Release")
            list(APPEND GLOBAL_CXX_OPTIONS "-O3")
        endif()

    elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
        list(APPEND GLOBAL_CXX_OPTIONS "-Wall")
        list(APPEND GLOBAL_CXX_OPTIONS "-Werror")
        list(APPEND GLOBAL_CXX_OPTIONS "-fPIC")
        list(APPEND GLOBAL_CXX_OPTIONS "-std=c++11")

        if(${CMAKE_BUILD_TYPE} MATCHES "Release")
            list(APPEND GLOBAL_CXX_OPTIONS "-O3")
        endif()

    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
        list(APPEND GLOBAL_CXX_OPTIONS "/WX")
        list(APPEND GLOBAL_CXX_OPTIONS "/MP")

        if(${CMAKE_BUILD_TYPE} MATCHES "Release")
            list(APPEND GLOBAL_CXX_OPTIONS "/Ox")
        endif()
    endif()

    target_compile_options(${targetName} PUBLIC ${GLOBAL_CXX_OPTIONS})

    set_property(TARGET ${targetName} PROPERTY CXX_STANDARD 11)

endfunction(setCommonCxxOptionsForTarget)


function(disableVisualStudioWarnings targetName)
    if(ISX_OS_WIN32)
        set_property(TARGET ${targetName} APPEND PROPERTY LINK_FLAGS /ignore:4099)
        # This a performance warning about casting from int to bool.
        # It could be placated by using x != 0, but it seems like a pretty silly warning.
        set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_FLAGS /wd4800)
    endif()
endfunction(disableVisualStudioWarnings)


function(setOsDefinesForTarget targetName)
    if(${ISX_OS_MACOS})
        target_compile_definitions(${targetName} PUBLIC ISX_OS_MACOS=1 ISX_OS_WIN32=0 ISX_OS_LINUX=0)
    elseif(${ISX_OS_WIN32})
        target_compile_definitions(${targetName} PUBLIC ISX_OS_MACOS=0 ISX_OS_WIN32=1 ISX_OS_LINUX=0)
    elseif(${ISX_OS_LINUX})
        target_compile_definitions(${targetName} PUBLIC ISX_OS_MACOS=0 ISX_OS_WIN32=0 ISX_OS_LINUX=1)
    endif()
    if(${ISX_ARCH_ARM})
        target_compile_definitions(${targetName} PUBLIC ISX_ARCH_ARM=1)
    else()
        target_compile_definitions(${targetName} PUBLIC ISX_ARCH_ARM=0)
    endif()
endfunction(setOsDefinesForTarget)


# The various of forms of file/directory copying in CMake are inconsistent
# and don't suit our purposes well.
# This function should be used to handle files and directories somewhat consistently.
# In the future we should rearrange things and use the install function more frequently
# because that's the "proper" way to do things in CMake.
function(installFiles TARGET_NAME DESTINATION FILES)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DESTINATION}
    )
    foreach(F ${FILES})
        get_filename_component(F_NAME ${F} NAME)
        if(IS_DIRECTORY ${F})
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${F} ${DESTINATION}/${F_NAME}
            )
        else()
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${F} ${DESTINATION}
            )
        endif()
    endforeach()
endfunction(installFiles)


function(installQtCoreSharedLibs TARGET_NAME DESTINATION)
    set(QT_CORE_SHARED_LIB_FILES "")
    if (${ISX_OS_MACOS})
        set(QT_CORE_SHARED_LIB_FILES
            ${QT_DIR}/lib/QtCore.framework
        )
    elseif(${ISX_OS_WIN32})
        if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
            set(QT_CORE_SHARED_LIB_FILES
                ${QT_DIR}/bin/Qt5Cored.dll
                ${QT_DIR}/bin/Qt5Cored.pdb
            )
        else()
            set(QT_CORE_SHARED_LIB_FILES
                ${QT_DIR}/bin/Qt5Core.dll
            )
        endif()
    elseif(${ISX_OS_LINUX})
        file(GLOB QT_CORE_SHARED_LIB_FILES
            ${QT_DIR}/lib/libQt5Core.so.5
        )
    endif()

    installFiles(${TARGET_NAME} ${DESTINATION} "${QT_CORE_SHARED_LIB_FILES}")
endfunction(installQtCoreSharedLibs)


function(installQtGuiSharedLibs TARGET_NAME DESTINATION)
    set(QT_GUI_SHARED_LIB_FILES "")
    if (${ISX_OS_MACOS})
        set(QT_GUI_SHARED_LIB_FILES
            ${QT_DIR}/lib/QtGui.framework
        )
    elseif(${ISX_OS_WIN32})
        if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
            set(QT_GUI_SHARED_LIB_FILES
                ${QT_DIR}/bin/Qt5Guid.dll
                ${QT_DIR}/bin/Qt5Guid.pdb
            )
        else()
            set(QT_GUI_SHARED_LIB_FILES
                ${QT_DIR}/bin/Qt5Gui.dll
            )
        endif()
    elseif(${ISX_OS_LINUX})
        file(GLOB QT_GUI_SHARED_LIB_FILES
            ${QT_DIR}/lib/libQt5Gui.so.5
        )
    endif()

    installFiles(${TARGET_NAME} ${DESTINATION} "${QT_GUI_SHARED_LIB_FILES}")
endfunction(installQtGuiSharedLibs)


function(installQtSharedLibs TARGET_NAME DESTINATION)
    if(${ISX_OS_WIN32})
        if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
            file(GLOB QT_SHARED_LIB_FILES ${QT_DIR}/bin/*d.*)
        else()
            file(GLOB QT_SHARED_LIB_FILES ${QT_DIR}/bin/*[a-c,e-z,0-9].dll)
        endif()
    elseif(${ISX_OS_LINUX})
        file(GLOB QT_SHARED_LIB_FILES ${QT_DIR}/lib/lib*)
    endif()

    installFiles(${TARGET_NAME} ${DESTINATION} "${QT_SHARED_LIB_FILES}")
endfunction(installQtSharedLibs)


function(installQtPlugins TARGET_NAME DESTINATION)
    set(QT_PLUGIN_FILES
        ${QT_DIR}/plugins/platforms
        ${QT_DIR}/plugins/imageformats
    )

    if(${ISX_OS_LINUX})
        list(APPEND QT_PLUGIN_FILES ${QT_DIR}/plugins/xcbglintegrations)
    endif()

    installFiles(${TARGET_NAME} ${DESTINATION} "${QT_PLUGIN_FILES}")
endfunction(installQtPlugins)


function(installFfmpegSharedLibs TARGET_NAME DESTINATION)
    if(${ISX_OS_WIN32})
        if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
            file(GLOB FFMPEG_SHARED_LIB_FILES
                ${FFMPEG_DIR}/lib/*.dll
                ${FFMPEG_DIR}/lib/*.pdb
            )
        else()
            file(GLOB FFMPEG_SHARED_LIB_FILES ${FFMPEG_DIR}/lib/*.dll)
        endif()
    else()
        set(FFMPEG_SHARED_LIB_FILES ${FFMPEG_LINK_LIBRARIES})
        if (${ISX_OS_MACOS})
            list(APPEND FFMPEG_SHARED_LIB_FILES "${FFMPEG_DIR}/lib/libswresample.2.dylib")
        elseif(${ISX_OS_LINUX})
            list(APPEND FFMPEG_SHARED_LIB_FILES "${FFMPEG_DIR}/lib/libswresample.so.2")
        endif()
    endif()

    installFiles(${TARGET_NAME} ${DESTINATION} "${FFMPEG_SHARED_LIB_FILES}")
endfunction(installFfmpegSharedLibs)


function(installMklSharedLibs TARGET_NAME DESTINATION)
    if(${ISX_OS_WIN32})
        set(MKL_SHARED_LIB_FILES )
        installFiles(${TARGET_NAME} ${DESTINATION} "${INTELMKL_DIR}/bin/tbb.dll")
    endif()
endfunction(installMklSharedLibs)


function(installFlexeraSharedLibs TARGET_NAME DESTINATION)
    installFiles(${TARGET_NAME} ${DESTINATION} "${FLEXNET_EMBEDDED_SHARED_LINK_LIBRARIES}")
endfunction(installFlexeraSharedLibs)

function(installScsSharedLibs TARGET_NAME DESTINATION)
    set(SCS_SHARED_LIB_FILES "")
    if(${ISX_OS_LINUX})
        file(GLOB SCS_SHARED_LIB_FILES
            ${SCS_DIR}/lib/liblapack.so.3
            ${SCS_DIR}/lib/libblas.so.3
            ${SCS_DIR}/lib/libgfortran.so.3
        )
    endif()

    installFiles(${TARGET_NAME} ${DESTINATION} "${SCS_SHARED_LIB_FILES}")
endfunction(installScsSharedLibs)
