tales_add_library(Common
    EH EHa
    SOURCES
        src/ChaAttr.cpp src/ChaMask.cpp src/CharacterRecord.cpp src/CommFunc.cpp src/EventRecord.cpp
        src/ForgeRecord.cpp src/HairRecord.cpp src/ItemAttr.cpp src/ItemContent.cpp src/ItemRecord.cpp
        src/JobInitEquip.cpp src/Kitbag.cpp src/LevelRecord.cpp src/LifeLvRecord.cpp src/LogFile.cpp
        src/MonRefRecord.cpp src/NpcRecord.cpp src/SailLvRecord.cpp src/ShipSet.cpp src/SkillRecord.cpp
        src/SkillStateRecord.cpp src/StringLib.cpp src/SwitchMapRecord.cpp src/TableData.cpp src/TerrainAttrib.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TALES_LIBRARIES_DIR}/CaLua/include
        ${TALES_LIBRARIES_DIR}/Server/sdk/include
        ${TALES_LIBRARIES_DIR}/Util/include
        ${TALES_LIBRARIES_DIR}/EncLib/include
        ${TALES_LIBRARIES_DIR}/ICUHelper/inc
        ${TALES_LIBRARIES_DIR}/icu/include
        ${TALES_LIBRARIES_DIR}/Cryptopp
    DEFINITIONS WINDOWS_IGNORE_PACKING_MISMATCH)
target_link_libraries(Common PUBLIC cryptlib)
