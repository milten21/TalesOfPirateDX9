add_executable(DumpBundle dump_bundle.cpp)
target_compile_options(DumpBundle PRIVATE /EHsc)
target_link_libraries(DumpBundle PRIVATE ICU::uc ICU::i18n ICU::data)
set_target_properties(DumpBundle PROPERTIES FOLDER Tools)
