add_executable(StructLayoutTests struct_layout_test.cpp)
target_include_directories(StructLayoutTests PRIVATE
    ${TALES_LIBRARIES_DIR}/common/include
    ${TALES_LIBRARIES_DIR}/Server/sdk/include
    ${TALES_LIBRARIES_DIR}/Util/include
    ${TALES_LIBRARIES_DIR}/InfoNet/include)
tales_target_eh(StructLayoutTests EHsc)
set_target_properties(StructLayoutTests PROPERTIES FOLDER Tests)
add_test(NAME StructLayout COMMAND StructLayoutTests)
