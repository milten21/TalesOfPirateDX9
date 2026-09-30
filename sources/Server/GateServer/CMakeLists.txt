tales_add_server(GateServer
    OPTIMIZE_LINK
    SOURCES src/GateServer.cpp src/main.cpp src/ToClient.cpp src/ToGameServer.cpp src/ToGroupServer.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${TALES_LIBRARIES_DIR}/Server/sdk/include
        ${TALES_LIBRARIES_DIR}/Common/include
        ${TALES_LIBRARIES_DIR}/Util/include
        ${TALES_LIBRARIES_DIR}/Status/Include
        ${TALES_LIBRARIES_DIR}/ICUHelper/inc
        ${TALES_LIBRARIES_DIR}/Cryptopp
    DEFINITIONS NARROW
    RELEASE_FLAGS /GS- /Oi /Oy /GT
    LIBRARIES
        LIBDBC util Status ICUHelper cryptlib Ws2_32 shlwapi winmm Psapi)
