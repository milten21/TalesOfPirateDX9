tales_add_server(AccountServer
    OPTIMIZE_LINK
    PCH ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h
    SOURCES
        src/AccountServer2.cpp src/BillService.cpp src/BTIService.cpp src/DataBaseCtrl.cpp src/GlobalVariable.cpp
        src/main.cpp src/MyThread.cpp src/stdafx.cpp src/AccountServer.rc
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${TALES_LIBRARIES_DIR}/Common/include
        ${TALES_LIBRARIES_DIR}/Util/include
        ${TALES_LIBRARIES_DIR}/Server/sdk/include
        ${TALES_LIBRARIES_DIR}/Ipss/include
        ${TALES_LIBRARIES_DIR}/ICUHelper/inc
        ${TALES_LIBRARIES_DIR}/icu/include
        ${TALES_LIBRARIES_DIR}/Cryptopp
    DEFINITIONS NOAUTHDLL
    LIBRARIES
        LIBDBC util ICUHelper ipss::bti icu::dt icu::uc icu::in Ws2_32 winmm)
