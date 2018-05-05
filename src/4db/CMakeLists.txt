
set( LIB_NAME ups-4db )

add_library( ${LIB_NAME} STATIC
    db.cc
    db_local.cc
    db_remote.cc
    histogram.cc
)

target_include_directories( ${LIB_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/src 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
    )

target_compile_options( ${LIB_NAME} PRIVATE ${ROMZ_CXX_FLAGS} )


