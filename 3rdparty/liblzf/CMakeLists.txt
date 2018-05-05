
set( LIB_NAME lzf )

add_library( ${LIB_NAME} STATIC
    lzf_c.c
    lzf_d.c
)

# target_include_directories( ${LIB_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src )

target_compile_options( ${LIB_NAME} PRIVATE 
    ${ROMZ_C_FLAGS}
    -Wno-implicit-fallthrough
)


