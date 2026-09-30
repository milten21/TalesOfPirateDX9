function(tales_import_icu name debug_lib release_lib debug_dll release_dll)
    add_library(icu::${name} SHARED IMPORTED GLOBAL)
    set_target_properties(icu::${name} PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
        IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/${debug_lib}
        IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/${release_lib}
        IMPORTED_LOCATION_DEBUG ${TALES_RUNTIME_DIR}/${debug_dll}
        IMPORTED_LOCATION_RELEASE ${TALES_RUNTIME_DIR}/${release_dll}
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)
endfunction()

tales_import_icu(uc icuucd.lib icuuc.lib icuuc38d.dll icuuc38.dll)
tales_import_icu(in icuind.lib icuin.lib icuin38d.dll icuin38.dll)
tales_import_icu(dt icudt.lib icudt.lib icudt38.dll icudt38.dll)
