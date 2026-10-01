if(NOT TALES_PLATFORM_LINUX)
    return()
endif()

add_executable(CrashHandlerTests crash_handler_test.cpp)
target_include_directories(CrashHandlerTests PRIVATE ${TALES_LIBRARIES_DIR}/Util/include)
target_compile_options(CrashHandlerTests PRIVATE -O0)
target_link_libraries(CrashHandlerTests PRIVATE util)
add_test(NAME CrashHandler.Linux COMMAND CrashHandlerTests)
