
set( LIB_NAME ups-3blob_manager )

add_library( ${LIB_NAME} STATIC
    blob_manager_disk.cc
    blob_manager_inmem.cc
)

target_include_directories( ${LIB_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/src 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
    )

target_compile_options( ${LIB_NAME} PRIVATE ${ROMZ_CXX_FLAGS} )


