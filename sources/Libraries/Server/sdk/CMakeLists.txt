tales_add_library(LIBDBC
    SOURCES
        src/AcceptConnect.cpp src/Comm.cpp src/CommRPC.cpp src/ConsoleInput.cpp src/DataSocket.cpp src/DBCCommon.cpp src/IniFile.cpp
        src/LogStream.cpp src/Packet.cpp src/PacketQueue.cpp src/Point.cpp src/Receiver.cpp src/RWMutex.cpp
        src/Sender.cpp src/ThreadPool.cpp src/Timer.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/Proj
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TALES_LIBRARIES_DIR}/Util/include
    DEFINITIONS _SERVER
    RELEASE_FLAGS /GS- /Oi /Ot /Oy /GT)
