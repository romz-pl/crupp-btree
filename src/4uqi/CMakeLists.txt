
set( LIB_NAME ups-4uqi )

add_library( ${LIB_NAME} STATIC
    parser.cc
    plugins.cc
    scanvisitorfactory.cc
    uqi.cc
)

target_include_directories( ${LIB_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/src 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
    )

target_compile_options( ${LIB_NAME} PRIVATE ${ROMZ_CXX_FLAGS} )


