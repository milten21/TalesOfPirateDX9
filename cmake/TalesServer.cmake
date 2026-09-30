function(tales_add_library name)
    cmake_parse_arguments(ARG "" "EH" "SOURCES;INCLUDES;DEFINITIONS;RELEASE_FLAGS" ${ARGN})
    if(NOT ARG_EH)
        set(ARG_EH EHsc)
    endif()
    add_library(${name} STATIC ${ARG_SOURCES})
    target_include_directories(${name} PRIVATE ${ARG_INCLUDES})
    target_compile_definitions(${name} PRIVATE _LIB ${ARG_DEFINITIONS})
    target_compile_options(${name} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/${ARG_EH}>")
    if(ARG_RELEASE_FLAGS)
        target_compile_options(${name} PRIVATE "$<$<CONFIG:Release>:${ARG_RELEASE_FLAGS}>")
    endif()
    set_target_properties(${name} PROPERTIES FOLDER Libraries)
endfunction()

function(tales_add_server name)
    cmake_parse_arguments(ARG "OPTIMIZE_LINK" "PCH" "SOURCES;INCLUDES;DEFINITIONS;LIBRARIES;RELEASE_FLAGS" ${ARGN})
    add_executable(${name} ${ARG_SOURCES})
    target_include_directories(${name} PRIVATE ${ARG_INCLUDES})
    target_compile_definitions(${name} PRIVATE _CONSOLE ${ARG_DEFINITIONS})
    target_compile_options(${name} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/EHa>")
    target_link_libraries(${name} PRIVATE ${ARG_LIBRARIES} odbc32 odbccp32)
    if(ARG_RELEASE_FLAGS)
        target_compile_options(${name} PRIVATE "$<$<CONFIG:Release>:${ARG_RELEASE_FLAGS}>")
    endif()
    if(ARG_OPTIMIZE_LINK)
        target_link_options(${name} PRIVATE "$<$<CONFIG:Release>:/OPT:REF;/OPT:ICF>")
    endif()
    if(ARG_PCH)
        target_precompile_headers(${name} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ARG_PCH}>")
    endif()

    set(bin_dir ${TALES_SOURCES_DIR}/Server/${name}/Bin)
    set_target_properties(${name} PROPERTIES
        DEBUG_POSTFIX _D
        FOLDER Servers
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${bin_dir}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${bin_dir}
        PDB_OUTPUT_DIRECTORY_DEBUG ${bin_dir}
        PDB_OUTPUT_DIRECTORY_RELEASE ${bin_dir}
        VS_DEBUGGER_WORKING_DIRECTORY ${TALES_SERVER_RUN_DIR}/${name})
endfunction()
