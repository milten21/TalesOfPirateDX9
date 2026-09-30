tales_add_server(GameServer
    PCH ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h
    SOURCES
        src/Action.cpp src/Attachable.cpp src/AttachManage.cpp src/Auction.cpp src/AuctionItem.cpp
        src/Birthplace.cpp src/cfl_iocp.cpp src/cfl_lock.cpp src/Character.cpp src/CharacterAI.cpp
        src/CharacterAIState.cpp src/CharacterCmd.cpp src/CharacterPacket.cpp src/CharacterPrl.cpp
        src/CharacterRun.cpp src/CharacterSuperCmd.cpp src/CharBoat.cpp src/CharForge.cpp src/CharScript.cpp
        src/CharStall.cpp src/CharTrade.cpp src/Config.cpp src/Entity.cpp src/EntityAlloc.cpp
        src/EntityScript.cpp src/EntitySpawn.cpp src/EventCalculate.cpp src/EventEntity.cpp
        src/EventHandler.cpp src/EyeshotCell.cpp src/FightAble.cpp src/FindPath.cpp src/GameApp.cpp
        src/GameAppNet.cpp src/GameConsole.cpp src/GameDB.cpp src/GameServerApp.cpp src/GameSMain.cpp src/gmsvr.cpp src/Guild.cpp
        src/HarmRec.cpp src/Item.cpp src/lua_gamectrl.cpp src/MapEntry.cpp src/MapRes.cpp src/MapTrigger.cpp
        src/Mission.cpp src/MoveAble.cpp src/NPC.cpp src/NpcScript.cpp src/Parser.cpp src/PicSet.cpp
        src/Picture.cpp src/Player.cpp src/Script.cpp src/StackWalker.cpp src/StateCell.cpp src/stdafx.cpp
        src/SubMap.cpp src/TradeLogDB.cpp src/Weather.cpp src/WorldEudemon.cpp
        src/xString.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${TALES_LIBRARIES_DIR}/Server/sdk/include
        ${TALES_LIBRARIES_DIR}/Common/include
        ${TALES_LIBRARIES_DIR}/Util/include
        ${TALES_LIBRARIES_DIR}/LuaJIT/src
        ${TALES_LIBRARIES_DIR}/ICUHelper/inc
        ${TALES_LIBRARIES_DIR}/InfoNet/include
        ${TALES_LIBRARIES_DIR}/Cryptopp
    DEFINITIONS __INFO_NET__ WINDOWS_IGNORE_PACKING_MISMATCH
    LIBRARIES
        luajit LIBDBC util Common ICUHelper InfoNet Ws2_32 winmm Psapi)
