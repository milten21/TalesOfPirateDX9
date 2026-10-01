add_executable(WireLayoutTests wire_layout_test.cpp)
target_include_directories(WireLayoutTests PRIVATE
    ${TALES_LIBRARIES_DIR}/common/include
    ${TALES_LIBRARIES_DIR}/Server/sdk/include
    ${TALES_LIBRARIES_DIR}/Util/include
    ${TALES_LIBRARIES_DIR}/InfoNet/include)
tales_target_eh(WireLayoutTests EHsc)
set_target_properties(WireLayoutTests PROPERTIES FOLDER Tests)
add_test(NAME WireLayout COMMAND WireLayoutTests)
