tales_add_library(InfoNet
    SOURCES src/InfoNet.cpp src/lock.cpp src/log.cpp src/thread.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include
    DEFINITIONS __INFO_NET__)
