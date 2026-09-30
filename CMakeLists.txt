cmake_minimum_required(VERSION 3.25)

project(TalesOfPirateServers LANGUAGES C CXX)

if(NOT MSVC)
    message(FATAL_ERROR "Only the MSVC toolchain is supported so far")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "The servers are 32-bit only; configure with -A Win32 or use the windows-x86 preset")
endif()

enable_language(ASM_MASM)

set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,Release>:ProgramDatabase>")

set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG")

add_compile_definitions(WIN32)
add_compile_options(/W0 "$<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/utf-8>" "$<$<COMPILE_LANGUAGE:CXX>:/std:c++latest>" "$<$<COMPILE_LANGUAGE:CXX>:/MP>")

set(TALES_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(TALES_LIBRARIES_DIR ${TALES_SOURCES_DIR}/Libraries)
set(TALES_RUNTIME_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/runtime/server)
set(TALES_SERVER_RUN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server)

include(cmake/TalesServer.cmake)

add_subdirectory(sources/Libraries/Cryptopp)
add_subdirectory(sources/Libraries/LuaJIT)
add_subdirectory(sources/Libraries/icu)
add_subdirectory(sources/Libraries/Server/sdk)
add_subdirectory(sources/Libraries/Util)
add_subdirectory(sources/Libraries/common)
add_subdirectory(sources/Libraries/ICUHelper)
add_subdirectory(sources/Libraries/Status)
add_subdirectory(sources/Libraries/InfoNet)

add_subdirectory(sources/Server/AccountServer)
add_subdirectory(sources/Server/GroupServer)
add_subdirectory(sources/Server/GateServer)
add_subdirectory(sources/Server/GameServer)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT GameServer)
