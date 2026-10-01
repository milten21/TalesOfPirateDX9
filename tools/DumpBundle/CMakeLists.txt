add_executable(DumpBundle dump_bundle.cpp)
tales_target_eh(DumpBundle EHsc)
target_link_libraries(DumpBundle PRIVATE ICU::uc ICU::i18n ICU::data)
set_target_properties(DumpBundle PROPERTIES FOLDER Tools)
