set(UTIL_SOURCES
    src/algo.cpp src/db.cpp src/db3.cpp src/LanguageRecord.cpp src/log.cpp src/log2.cpp src/logutil.cpp
    src/MapDataVer.cpp src/MapUtil.cpp src/md5c.c src/pch.cpp src/SectionData.cpp src/util2.cpp)

if(TALES_PLATFORM_WINDOWS)
    list(APPEND UTIL_SOURCES src/ErrorHandler.cpp src/se_translator.cpp src/Stacktrace.cpp src/sym_engine.cpp)
else()
    list(APPEND UTIL_SOURCES src/CrashHandlerLinux.cpp)
endif()

tales_add_library(util
    EH EHa
    SOURCES ${UTIL_SOURCES}
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TALES_LIBRARIES_DIR}/SDL-1.2.7/include
        ${TALES_LIBRARIES_DIR}/EncLib/include)

if(TALES_PLATFORM_LINUX)
    target_link_libraries(util PUBLIC ODBC::ODBC)
endif()
