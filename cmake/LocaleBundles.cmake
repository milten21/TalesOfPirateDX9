function(tales_add_locale_bundle server locale)
    set(dir ${TALES_SERVER_RUN_DIR}/${server})
    add_custom_command(
        OUTPUT ${dir}/${locale}.res
        COMMAND ${ICU_GENRB_EXECUTABLE} -q -e UTF-8 -d ${dir} ${dir}/${locale}.txt
        DEPENDS ${dir}/${locale}.txt
        VERBATIM)
    set_property(GLOBAL APPEND PROPERTY TALES_LOCALE_BUNDLES ${dir}/${locale}.res)
endfunction()

function(tales_finish_locale_bundles)
    get_property(bundles GLOBAL PROPERTY TALES_LOCALE_BUNDLES)
    add_custom_target(LocaleBundles ALL DEPENDS ${bundles})
    set_target_properties(LocaleBundles PROPERTIES FOLDER Libraries)
endfunction()
