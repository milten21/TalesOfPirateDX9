set(LJ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LJ_DYNASM ${CMAKE_CURRENT_SOURCE_DIR}/dynasm)
set(LJ_GEN ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${LJ_GEN})

if(TALES_PLATFORM_WINDOWS)
    set(CMAKE_C_FLAGS_DEBUG "/O2 /Ob2")
    set(LJ_DASM_FLAGS -D WIN -D JIT -D FFI)
    set(LJ_VM_MODE peobj)
    set(LJ_VM ${LJ_GEN}/lj_vm.obj)
    set(LJ_HOST_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
else()
    set(LJ_DASM_FLAGS -D JIT -D FFI)
    set(LJ_VM_MODE elfasm)
    set(LJ_VM ${LJ_GEN}/lj_vm.S)
    set(LJ_HOST_DEFINITIONS _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE)
endif()

set(LJ_ALL_LIB
    lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c
    lib_ffi.c)

add_executable(luajit_minilua ${LJ_SRC}/host/minilua.c)
target_compile_definitions(luajit_minilua PRIVATE ${LJ_HOST_DEFINITIONS})
if(TALES_PLATFORM_LINUX)
    target_link_libraries(luajit_minilua PRIVATE m)
endif()

add_custom_command(
    OUTPUT ${LJ_GEN}/buildvm_arch.h
    COMMAND luajit_minilua ${LJ_DYNASM}/dynasm.lua -LN ${LJ_DASM_FLAGS} -o ${LJ_GEN}/buildvm_arch.h ${LJ_SRC}/vm_x86.dasc
    DEPENDS luajit_minilua ${LJ_SRC}/vm_x86.dasc ${LJ_DYNASM}/dynasm.lua ${LJ_DYNASM}/dasm_x86.lua
    VERBATIM)

add_executable(luajit_buildvm
    ${LJ_SRC}/host/buildvm.c
    ${LJ_SRC}/host/buildvm_asm.c
    ${LJ_SRC}/host/buildvm_fold.c
    ${LJ_SRC}/host/buildvm_lib.c
    ${LJ_SRC}/host/buildvm_peobj.c
    ${LJ_GEN}/buildvm_arch.h)
target_include_directories(luajit_buildvm PRIVATE ${LJ_SRC} ${LJ_DYNASM} ${LJ_GEN})
target_compile_definitions(luajit_buildvm PRIVATE ${LJ_HOST_DEFINITIONS})

set(LJ_GENERATED ${LJ_VM})
add_custom_command(
    OUTPUT ${LJ_VM}
    COMMAND luajit_buildvm -m ${LJ_VM_MODE} -o ${LJ_VM}
    DEPENDS luajit_buildvm
    VERBATIM)

foreach(mode bcdef ffdef libdef recdef)
    add_custom_command(
        OUTPUT ${LJ_GEN}/lj_${mode}.h
        COMMAND luajit_buildvm -m ${mode} -o ${LJ_GEN}/lj_${mode}.h ${LJ_ALL_LIB}
        WORKING_DIRECTORY ${LJ_SRC}
        DEPENDS luajit_buildvm
        VERBATIM)
    list(APPEND LJ_GENERATED ${LJ_GEN}/lj_${mode}.h)
endforeach()

add_custom_command(
    OUTPUT ${LJ_GEN}/lj_folddef.h
    COMMAND luajit_buildvm -m folddef -o ${LJ_GEN}/lj_folddef.h lj_opt_fold.c
    WORKING_DIRECTORY ${LJ_SRC}
    DEPENDS luajit_buildvm ${LJ_SRC}/lj_opt_fold.c
    VERBATIM)
list(APPEND LJ_GENERATED ${LJ_GEN}/lj_folddef.h)

file(GLOB LJ_SOURCES CONFIGURE_DEPENDS ${LJ_SRC}/lj_*.c ${LJ_SRC}/lib_*.c)
if(TALES_PLATFORM_WINDOWS)
    set_source_files_properties(${LJ_VM} PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
else()
    set_source_files_properties(${LJ_VM} PROPERTIES LANGUAGE ASM GENERATED TRUE)
endif()

add_library(luajit STATIC ${LJ_SOURCES} ${LJ_GENERATED})
target_include_directories(luajit PUBLIC ${LJ_SRC})
target_include_directories(luajit PRIVATE ${LJ_GEN})
target_compile_definitions(luajit PRIVATE ${LJ_HOST_DEFINITIONS})
if(TALES_PLATFORM_LINUX)
    target_compile_options(luajit PRIVATE -fomit-frame-pointer -U_FORTIFY_SOURCE)
    target_link_libraries(luajit PUBLIC m ${CMAKE_DL_LIBS})
endif()

set_target_properties(luajit luajit_minilua luajit_buildvm PROPERTIES FOLDER Libraries/ThirdParty)
