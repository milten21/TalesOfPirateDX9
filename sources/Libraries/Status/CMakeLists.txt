set(STATUS_SOURCES Src/UdpManage.cpp Src/UdpSocket.cpp)
if(TALES_PLATFORM_WINDOWS)
    list(APPEND STATUS_SOURCES Src/UdpClient.cpp)
endif()

tales_add_library(Status
    SOURCES ${STATUS_SOURCES}
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/Include ${TALES_LIBRARIES_DIR}/Server/sdk/include)
