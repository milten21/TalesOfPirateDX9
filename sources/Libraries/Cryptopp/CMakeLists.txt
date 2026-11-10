set(CRYPTOPP_SOURCES
    cryptlib.cpp cpu.cpp integer.cpp 3way.cpp adler32.cpp algebra.cpp algparam.cpp allocate.cpp arc4.cpp aria.cpp
    aria_simd.cpp ariatab.cpp asn.cpp authenc.cpp base32.cpp base64.cpp basecode.cpp bfinit.cpp blake2.cpp
    blake2s_simd.cpp blake2b_simd.cpp blowfish.cpp blumshub.cpp camellia.cpp cast.cpp casts.cpp cbcmac.cpp ccm.cpp
    chacha.cpp chacha_simd.cpp chacha_avx.cpp chachapoly.cpp cham.cpp cham_simd.cpp channels.cpp cmac.cpp crc.cpp
    crc_simd.cpp darn.cpp default.cpp des.cpp dessp.cpp dh.cpp dh2.cpp dll.cpp donna_32.cpp donna_64.cpp
    donna_sse.cpp dsa.cpp eax.cpp ec2n.cpp ecp.cpp elgamal.cpp emsa2.cpp esign.cpp files.cpp filters.cpp fips140.cpp
    fipstest.cpp gcm.cpp gcm_simd.cpp gf256.cpp gf2_32.cpp gf2n.cpp gf2n_simd.cpp gfpcrypt.cpp gost.cpp gzip.cpp
    hc128.cpp hc256.cpp hex.cpp hight.cpp hmac.cpp hrtimer.cpp ida.cpp idea.cpp iterhash.cpp kalyna.cpp
    kalynatab.cpp keccak.cpp keccak_core.cpp keccak_simd.cpp lea.cpp lea_simd.cpp luc.cpp mars.cpp marss.cpp md2.cpp
    md4.cpp md5.cpp misc.cpp modes.cpp mqueue.cpp mqv.cpp nbtheory.cpp oaep.cpp osrng.cpp padlkrng.cpp panama.cpp
    pch.cpp pkcspad.cpp poly1305.cpp polynomi.cpp pssr.cpp pubkey.cpp queue.cpp rabin.cpp randpool.cpp rabbit.cpp
    rc2.cpp rc5.cpp rc6.cpp rdrand.cpp rdtables.cpp rijndael.cpp rijndael_simd.cpp ripemd.cpp rng.cpp rsa.cpp rw.cpp
    safer.cpp salsa.cpp scrypt.cpp seal.cpp seed.cpp serpent.cpp sha.cpp sha_simd.cpp sha3.cpp shacal2.cpp
    shacal2_simd.cpp shake.cpp shark.cpp sharkbox.cpp simeck.cpp simon.cpp simon128_simd.cpp simple.cpp skipjack.cpp
    sm3.cpp sm4.cpp sm4_simd.cpp sosemanuk.cpp speck.cpp speck128_simd.cpp square.cpp squaretb.cpp sse_simd.cpp
    strciphr.cpp tea.cpp tftables.cpp threefish.cpp tiger.cpp tigertab.cpp ttmac.cpp tweetnacl.cpp twofish.cpp
    vmac.cpp wake.cpp whrlpool.cpp xed25519.cpp xtr.cpp xtrcrypt.cpp xts.cpp zdeflate.cpp zinflate.cpp zlib.cpp)

if(TALES_PLATFORM_WINDOWS)
    set(CRYPTOPP_ASM_SOURCES rdrand.asm rdseed.asm)
    set_source_files_properties(${CRYPTOPP_ASM_SOURCES} PROPERTIES COMPILE_OPTIONS "/W3;/Cx;/safeseh" COMPILE_DEFINITIONS _M_X86)
    set_source_files_properties(chacha_avx.cpp PROPERTIES COMPILE_OPTIONS /arch:AVX2)
else()
    set(CRYPTOPP_ASM_SOURCES)
    set_source_files_properties(chacha_simd.cpp donna_sse.cpp sse_simd.cpp PROPERTIES COMPILE_OPTIONS -msse2)
    set_source_files_properties(aria_simd.cpp cham_simd.cpp keccak_simd.cpp lea_simd.cpp simon128_simd.cpp speck128_simd.cpp
        PROPERTIES COMPILE_OPTIONS -mssse3)
    set_source_files_properties(blake2b_simd.cpp blake2s_simd.cpp PROPERTIES COMPILE_OPTIONS -msse4.1)
    set_source_files_properties(crc_simd.cpp PROPERTIES COMPILE_OPTIONS -msse4.2)
    set_source_files_properties(gcm_simd.cpp PROPERTIES COMPILE_OPTIONS "-mssse3;-mpclmul")
    set_source_files_properties(gf2n_simd.cpp PROPERTIES COMPILE_OPTIONS -mpclmul)
    set_source_files_properties(rijndael_simd.cpp PROPERTIES COMPILE_OPTIONS "-msse4.1;-maes")
    set_source_files_properties(sm4_simd.cpp PROPERTIES COMPILE_OPTIONS "-mssse3;-maes")
    set_source_files_properties(chacha_avx.cpp PROPERTIES COMPILE_OPTIONS -mavx2)
    set_source_files_properties(sha_simd.cpp shacal2_simd.cpp PROPERTIES COMPILE_OPTIONS "-msse4.2;-msha")
endif()

add_library(cryptlib STATIC ${CRYPTOPP_SOURCES} ${CRYPTOPP_ASM_SOURCES})
target_include_directories(cryptlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(TALES_PLATFORM_WINDOWS)
    target_compile_options(cryptlib PRIVATE
        "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:/Oi;/GF;/Gy;/Oy>")
else()
    target_link_libraries(cryptlib PUBLIC Threads::Threads)
endif()
set_target_properties(cryptlib PROPERTIES FOLDER Libraries/ThirdParty)
