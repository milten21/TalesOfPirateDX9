add_executable(ICUBytesCheck icu_bytes.cpp)
target_include_directories(ICUBytesCheck PRIVATE ${TALES_LIBRARIES_DIR}/ICUHelper/inc)
target_compile_options(ICUBytesCheck PRIVATE /EHa)
target_link_libraries(ICUBytesCheck PRIVATE ICUHelper)
add_dependencies(ICUBytesCheck LocaleBundles)
set_target_properties(ICUBytesCheck PROPERTIES FOLDER Tests)

foreach(server AccountServer GameServer GateServer GroupServer)
    add_test(NAME ICUBytes.${server}
        COMMAND ICUBytesCheck ${server} ${TALES_SERVER_RUN_DIR}/${server} ${CMAKE_CURRENT_SOURCE_DIR}/golden/${server}.txt)
endforeach()

add_executable(ICUHelperTests icu_helper_tests.cpp)
target_include_directories(ICUHelperTests PRIVATE ${TALES_LIBRARIES_DIR}/ICUHelper/inc)
target_compile_options(ICUHelperTests PRIVATE /EHa)
target_link_libraries(ICUHelperTests PRIVATE ICUHelper)
add_dependencies(ICUHelperTests LocaleBundles)
set_target_properties(ICUHelperTests PROPERTIES FOLDER Tests)
add_test(NAME ICUHelper.Unit COMMAND ICUHelperTests ${TALES_SERVER_RUN_DIR}/GameServer)
