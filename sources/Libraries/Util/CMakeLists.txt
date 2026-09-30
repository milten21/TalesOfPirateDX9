tales_add_library(util
    EH EHa
    SOURCES
        src/algo.cpp src/db.cpp src/db3.cpp src/ErrorHandler.cpp src/LanguageRecord.cpp src/log.cpp
        src/log2.cpp src/logutil.cpp src/MapDataVer.cpp src/MapUtil.cpp src/md5c.c src/pch.cpp
        src/SectionData.cpp src/se_translator.cpp src/Stacktrace.cpp src/sym_engine.cpp src/util2.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TALES_LIBRARIES_DIR}/SDL-1.2.7/include
        ${TALES_LIBRARIES_DIR}/EncLib/include)
