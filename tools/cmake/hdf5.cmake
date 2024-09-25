set(HDF5_HEADER_SEARCH_PATHS)
set(HDF5_LINK_LIBRARIES)
set(t ${THIRD_PARTY_DIR})
string(APPEND t "/hdf5/1.10")

if(${ISX_OS_MACOS})
    if(${CMAKE_BUILD_TYPE} MATCHES "Debug")

        set(a ${t})
        string(APPEND a "/osx/lib/libhdf5_cpp_debug.a")
        list(APPEND HDF5_LINK_LIBRARIES ${a})

        set(a ${t})
        string(APPEND a "/osx/lib/libhdf5_debug.a")
        list(APPEND HDF5_LINK_LIBRARIES ${a})

    else()

        set(a ${t})
        string(APPEND a "/osx/lib/libhdf5_cpp-static.a")
        list(APPEND HDF5_LINK_LIBRARIES ${a})

        set(a ${t})
        string(APPEND a "/osx/lib/libhdf5-static.a")
        list(APPEND HDF5_LINK_LIBRARIES ${a})

    endif()

    set(a ${t})
    string(APPEND a "/osx/include")
    list(APPEND HDF5_HEADER_SEARCH_PATHS ${a})

elseif(${ISX_OS_LINUX})

    set(HDF5_DIR "${t}/linux")

    if(${ISX_ARCH_ARM})
        string(APPEND HDF5_DIR "-arm")
        set(HDF5_LIB_DIR "${HDF5_DIR}/lib")
    else()
        set(HDF5_LIB_DIR "${HDF5_DIR}/lib-fPIC")
    endif()

    list(APPEND HDF5_LINK_LIBRARIES "${HDF5_LIB_DIR}/libhdf5_cpp.a")
    list(APPEND HDF5_LINK_LIBRARIES "${HDF5_LIB_DIR}/libhdf5.a")

    # dynamically linking to system libdl
    list(APPEND HDF5_LINK_LIBRARIES "libdl.so")

    list(APPEND HDF5_HEADER_SEARCH_PATHS "${HDF5_DIR}/include")

elseif(${ISX_OS_WIN32})
    if(${CMAKE_BUILD_TYPE} MATCHES "Debug")

        set(a ${t})
        string(APPEND a "/win/lib/libhdf5_cpp_D.lib")
        list(APPEND HDF5_LINK_LIBRARIES ${a})

        set(a ${t})
        string(APPEND a "/win/lib/libhdf5_D.lib")
        list(APPEND HDF5_LINK_LIBRARIES ${a})

    else()

        set(a ${t})
        string(APPEND a "/win/lib/libhdf5_cpp.lib")
        list(APPEND HDF5_LINK_LIBRARIES ${a})

        set(a ${t})
        string(APPEND a "/win/lib/libhdf5.lib")
        list(APPEND HDF5_LINK_LIBRARIES ${a})

    endif()

    set(a ${t})
    string(APPEND a "/win/include")
    list(APPEND HDF5_HEADER_SEARCH_PATHS ${a})

endif()
