tales_add_library(ICUHelper
    EH EHa
    SOURCES src/pi_Alloc.cpp src/pi_Memory.cpp src/ResourceBundleManage.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${TALES_LIBRARIES_DIR}/icu/include)
