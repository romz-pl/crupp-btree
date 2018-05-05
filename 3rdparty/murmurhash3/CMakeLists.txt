
set( LIB_NAME MurmurHash3 )

add_library( ${LIB_NAME} STATIC
    MurmurHash3.cc
)

# target_include_directories( ${LIB_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src )

target_compile_options( ${LIB_NAME} PRIVATE 
    ${ROMZ_CXX_FLAGS} 
    -Wno-implicit-fallthrough
)


