set(BOOST_HEADER_SEARCH_PATHS)
set(BOOST_LINK_LIBRARIES)
set(t ${THIRD_PARTY_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_MATH_DISABLE_FLOAT128")
string(APPEND t "/boost/1.72.0")

if(${ISX_OS_WIN32})

    set(BOOST_DIR "${t}/win")
    list(APPEND BOOST_HEADER_SEARCH_PATHS "${BOOST_DIR}/include")

    set(BOOST_LIB_DIR "${BOOST_DIR}/lib")
    list(APPEND BOOST_LINK_LIBRARIES "${BOOST_LIB_DIR}/libboost_serialization-vc140-mt-x64-1_72.lib")
    list(APPEND BOOST_LINK_LIBRARIES "${BOOST_LIB_DIR}/libboost_program_options-vc140-mt-x64-1_72.lib")

elseif(${ISX_OS_LINUX})

    set(BOOST_DIR "${t}/linux")
    list(APPEND BOOST_HEADER_SEARCH_PATHS "${BOOST_DIR}/include")

    set(BOOST_LIB_DIR "${BOOST_DIR}/lib")
    list(APPEND BOOST_LINK_LIBRARIES "${BOOST_LIB_DIR}/libboost_serialization.a")
    list(APPEND BOOST_LINK_LIBRARIES "${BOOST_LIB_DIR}/libboost_program_options.a")

elseif(${ISX_OS_MACOS})

    set(BOOST_DIR "${t}/osx")
    list(APPEND BOOST_HEADER_SEARCH_PATHS "${BOOST_DIR}/include")

    set(BOOST_LIB_DIR "${BOOST_DIR}/lib")
    list(APPEND BOOST_LINK_LIBRARIES "${BOOST_LIB_DIR}/libboost_serialization.a")
    list(APPEND BOOST_LINK_LIBRARIES "${BOOST_LIB_DIR}/libboost_program_options.a")


endif()