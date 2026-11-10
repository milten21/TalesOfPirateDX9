tales_add_server(GateServer
    OPTIMIZE_LINK
    SOURCES src/GateServer.cpp src/main.cpp src/ToClient.cpp src/ToGameServer.cpp src/ToGroupServer.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${TALES_LIBRARIES_DIR}/Server/sdk/include
        ${TALES_LIBRARIES_DIR}/common/include
        ${TALES_LIBRARIES_DIR}/Util/include
        ${TALES_LIBRARIES_DIR}/Status/Include
        ${TALES_LIBRARIES_DIR}/ICUHelper/inc
        ${TALES_LIBRARIES_DIR}/Cryptopp
    DEFINITIONS NARROW
    RELEASE_FLAGS /GS- /Oi /Oy /GT
    LIBRARIES
        LIBDBC util Status ICUHelper cryptlib)
if(TALES_PLATFORM_WINDOWS)
    target_link_libraries(GateServer PRIVATE shlwapi Psapi)
endif()
