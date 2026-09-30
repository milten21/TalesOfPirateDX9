tales_add_library(ICUHelper
    EH EHa
    SOURCES src/pi_Alloc.cpp src/pi_Memory.cpp src/ResourceBundleManage.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(ICUHelper PUBLIC ICU::uc ICU::i18n ICU::data)
