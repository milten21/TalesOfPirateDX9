cmake_minimum_required(VERSION 3.25)

set(VCPKG_TARGET_TRIPLET x86-windows-static CACHE STRING "")
set(VCPKG_HOST_TRIPLET x64-windows CACHE STRING "")
set(VCPKG_APPLOCAL_DEPS OFF CACHE BOOL "")
if(NOT CMAKE_TOOLCHAIN_FILE)
    if(NOT DEFINED ENV{VCPKG_ROOT})
        message(FATAL_ERROR "Set VCPKG_ROOT to a vcpkg checkout (git clone https://github.com/microsoft/vcpkg); it supplies ICU through vcpkg.json")
    endif()
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

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
set(TALES_SERVER_RUN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server)

include(cmake/TalesServer.cmake)
include(cmake/LocaleBundles.cmake)

find_package(ICU REQUIRED COMPONENTS uc i18n data)
find_program(ICU_GENRB_EXECUTABLE genrb
    PATHS
        ${VCPKG_INSTALLED_DIR}/${VCPKG_HOST_TRIPLET}/tools/icu/bin
        ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/icu/bin
    REQUIRED)

enable_testing()

add_subdirectory(sources/Libraries/Cryptopp)
add_subdirectory(sources/Libraries/LuaJIT)
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

tales_add_locale_bundle(AccountServer en_US)
tales_add_locale_bundle(AccountServer ru_RU)
tales_add_locale_bundle(GameServer en_US)
tales_add_locale_bundle(GateServer en_US)
tales_add_locale_bundle(GroupServer en_US)
tales_finish_locale_bundles()

add_subdirectory(tools/DumpBundle)
add_subdirectory(tests/ICUHelper)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT GameServer)
