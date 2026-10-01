cmake_minimum_required(VERSION 3.25)

if(CMAKE_HOST_WIN32)
    set(VCPKG_TARGET_TRIPLET x86-windows-static CACHE STRING "")
    set(VCPKG_HOST_TRIPLET x64-windows CACHE STRING "")
    set(VCPKG_APPLOCAL_DEPS OFF CACHE BOOL "")
    if(NOT CMAKE_TOOLCHAIN_FILE)
        if(NOT DEFINED ENV{VCPKG_ROOT})
            message(FATAL_ERROR "Set VCPKG_ROOT to a vcpkg checkout (git clone https://github.com/microsoft/vcpkg); it supplies ICU through vcpkg.json")
        endif()
        set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
    endif()
endif()

project(TalesOfPirateServers LANGUAGES C CXX)

if(MSVC)
    set(TALES_PLATFORM_WINDOWS ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(TALES_PLATFORM_LINUX ON)
else()
    message(FATAL_ERROR "Supported toolchains: MSVC on Windows and GCC on Linux")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "The servers are 32-bit only; on Windows configure with -A Win32 or use the windows-x86 preset, on Linux build in the i386 image (see Dockerfile)")
endif()

set(TALES_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(TALES_LIBRARIES_DIR ${TALES_SOURCES_DIR}/Libraries)
set(TALES_SERVER_RUN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server)

if(TALES_PLATFORM_WINDOWS)
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

    set(TALES_LOCALE_DIR ${TALES_SERVER_RUN_DIR})
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
    endif()

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS ON)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    add_compile_options(-fno-strict-aliasing -finput-charset=UTF-8 -fexec-charset=UTF-8)
    add_link_options(-rdynamic)

    enable_language(ASM)

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    find_package(ODBC REQUIRED)

    set(TALES_LOCALE_DIR ${CMAKE_BINARY_DIR}/locale)
endif()

include(cmake/TalesServer.cmake)
include(cmake/LocaleBundles.cmake)

find_package(ICU REQUIRED COMPONENTS uc i18n data)
find_program(ICU_GENRB_EXECUTABLE genrb
    PATHS
        ${VCPKG_INSTALLED_DIR}/${VCPKG_HOST_TRIPLET}/tools/icu/bin
        ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/icu/bin
    REQUIRED)

enable_testing()

add_subdirectory(sources/Libraries/Server/sdk)
add_subdirectory(sources/Libraries/Util)
add_subdirectory(sources/Libraries/ICUHelper)

add_subdirectory(sources/Libraries/Cryptopp)
add_subdirectory(sources/Libraries/LuaJIT)
add_subdirectory(sources/Libraries/common)
add_subdirectory(sources/Libraries/Status)

add_subdirectory(sources/Server/AccountServer)
add_subdirectory(sources/Server/GroupServer)
add_subdirectory(sources/Server/GateServer)

if(TALES_PLATFORM_WINDOWS)
    add_subdirectory(sources/Libraries/InfoNet)
    add_subdirectory(sources/Server/GameServer)
endif()

tales_add_locale_bundle(AccountServer en_US)
tales_add_locale_bundle(AccountServer ru_RU)
tales_add_locale_bundle(GameServer en_US)
tales_add_locale_bundle(GateServer en_US)
tales_add_locale_bundle(GroupServer en_US)
tales_finish_locale_bundles()

add_subdirectory(tools/DumpBundle)
add_subdirectory(tests/ICUHelper)
add_subdirectory(tests/CrashHandler)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT GameServer)
