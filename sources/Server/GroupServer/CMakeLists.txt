tales_add_server(GroupServer
    OPTIMIZE_LINK
    PCH ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h
    SOURCES
        src/DBConnect.cpp src/Friend.cpp src/GroupServerApp.cpp src/GroupServerAppChat.cpp
        src/GroupServerAppFrnd.cpp src/GroupServerAppGuld.cpp src/GroupServerAppInit.cpp
        src/GroupServerAppMaster.cpp src/GroupServerAppPrsn.cpp src/GroupServerAppServ.cpp
        src/GroupServerAppTeam.cpp src/GroupSMain.cpp src/Guild.cpp src/GuildMember.cpp src/Invitation.cpp
        src/LuaFunc.cpp src/Master.cpp src/Parser.cpp src/Player.cpp src/Script.cpp src/SessionChat.cpp
        src/stdafx.cpp src/Team.cpp src/TeamMember.cpp
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${TALES_LIBRARIES_DIR}/Util/include
        ${TALES_LIBRARIES_DIR}/Server/sdk/include
        ${TALES_LIBRARIES_DIR}/Common/include
        ${TALES_LIBRARIES_DIR}/LuaJIT/src
        ${TALES_LIBRARIES_DIR}/ICUHelper/inc
        ${TALES_LIBRARIES_DIR}/Cryptopp
    LIBRARIES
        luajit LIBDBC util Common ICUHelper Ws2_32 winmm)
