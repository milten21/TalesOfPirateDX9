set(LJ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LJ_DYNASM ${CMAKE_CURRENT_SOURCE_DIR}/dynasm)
set(LJ_GEN ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${LJ_GEN})

set(CMAKE_C_FLAGS_DEBUG "/O2 /Ob2")

set(LJ_ALL_LIB
    lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c
    lib_ffi.c)

add_executable(luajit_minilua ${LJ_SRC}/host/minilua.c)
target_compile_definitions(luajit_minilua PRIVATE _CRT_SECURE_NO_DEPRECATE)

add_custom_command(
    OUTPUT ${LJ_GEN}/buildvm_arch.h
    COMMAND luajit_minilua ${LJ_DYNASM}/dynasm.lua -LN -D WIN -D JIT -D FFI -o ${LJ_GEN}/buildvm_arch.h ${LJ_SRC}/vm_x86.dasc
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
target_compile_definitions(luajit_buildvm PRIVATE _CRT_SECURE_NO_DEPRECATE)

set(LJ_GENERATED ${LJ_GEN}/lj_vm.obj)
add_custom_command(
    OUTPUT ${LJ_GEN}/lj_vm.obj
    COMMAND luajit_buildvm -m peobj -o ${LJ_GEN}/lj_vm.obj
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
set_source_files_properties(${LJ_GEN}/lj_vm.obj PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)

add_library(luajit STATIC ${LJ_SOURCES} ${LJ_GENERATED})
target_include_directories(luajit PUBLIC ${LJ_SRC})
target_include_directories(luajit PRIVATE ${LJ_GEN})
target_compile_definitions(luajit PRIVATE _CRT_SECURE_NO_DEPRECATE)

set_target_properties(luajit luajit_minilua luajit_buildvm PROPERTIES FOLDER Libraries/ThirdParty)
