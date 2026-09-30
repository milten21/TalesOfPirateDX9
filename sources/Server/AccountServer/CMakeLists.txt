tales_add_server(AccountServer
    OPTIMIZE_LINK
    PCH ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h
    SOURCES
        src/AccountServer2.cpp src/DataBaseCtrl.cpp src/GlobalVariable.cpp src/main.cpp src/MainQueue.cpp
        src/MyThread.cpp src/stdafx.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${TALES_LIBRARIES_DIR}/Common/include
        ${TALES_LIBRARIES_DIR}/Util/include
        ${TALES_LIBRARIES_DIR}/Server/sdk/include
        ${TALES_LIBRARIES_DIR}/ICUHelper/inc
        ${TALES_LIBRARIES_DIR}/icu/include
        ${TALES_LIBRARIES_DIR}/Cryptopp
    DEFINITIONS NOAUTHDLL
    LIBRARIES
        LIBDBC util ICUHelper icu::dt icu::uc icu::in Ws2_32 winmm)
