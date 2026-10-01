function(tales_add_locale_bundle server locale)
    set(source_dir ${TALES_SERVER_RUN_DIR}/${server})
    set(dir ${TALES_LOCALE_DIR}/${server})
    add_custom_command(
        OUTPUT ${dir}/${locale}.res
        COMMAND ${CMAKE_COMMAND} -E make_directory ${dir}
        COMMAND ${ICU_GENRB_EXECUTABLE} -q -e UTF-8 -d ${dir} ${source_dir}/${locale}.txt
        DEPENDS ${source_dir}/${locale}.txt
        VERBATIM)
    set_property(GLOBAL APPEND PROPERTY TALES_LOCALE_BUNDLES ${dir}/${locale}.res)
    if(TALES_PLATFORM_LINUX)
        install(FILES ${dir}/${locale}.res DESTINATION ${server})
    endif()

    get_property(servers GLOBAL PROPERTY TALES_LOCALE_SERVERS)
    if(server IN_LIST servers OR dir STREQUAL source_dir)
        return()
    endif()
    set_property(GLOBAL APPEND PROPERTY TALES_LOCALE_SERVERS ${server})
    add_custom_command(
        OUTPUT ${dir}/${server}.loc
        COMMAND ${CMAKE_COMMAND} -E copy ${source_dir}/${server}.loc ${dir}/${server}.loc
        DEPENDS ${source_dir}/${server}.loc
        VERBATIM)
    set_property(GLOBAL APPEND PROPERTY TALES_LOCALE_BUNDLES ${dir}/${server}.loc)
    if(TALES_PLATFORM_LINUX)
        install(FILES ${dir}/${server}.loc DESTINATION ${server})
    endif()
endfunction()

function(tales_finish_locale_bundles)
    get_property(bundles GLOBAL PROPERTY TALES_LOCALE_BUNDLES)
    add_custom_target(LocaleBundles ALL DEPENDS ${bundles})
    set_target_properties(LocaleBundles PROPERTIES FOLDER Libraries)
endfunction()
