
set( LIB_NAME ups-3btree )

add_library( ${LIB_NAME} STATIC
    btree_check.cc
    btree_cursor.cc
    btree_erase.cc
    btree_find.cc
    btree_index.cc
    btree_insert.cc
    btree_stats.cc
    btree_update.cc
    btree_visit.cc
)

target_include_directories( ${LIB_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/src 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
    )

target_compile_options( ${LIB_NAME} PRIVATE ${ROMZ_CXX_FLAGS} )


