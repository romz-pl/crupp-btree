set( TEST_NAME btree_test )

add_executable( ${TEST_NAME}
    #aes.cpp
    api110.cpp
    approx.cpp
    blob_manager.cpp
    btree.cpp
    btree_cursor.cpp
    btree_default.cpp
    btree_erase.cpp
    btree_insert.cpp
    btree_key.cpp
    changeset.cpp
    check.cpp
    compression.cpp
    cppapi.cpp
    crc32.cpp
    cursor1.cpp
    cursor2.cpp
    db.cpp
    device.cpp
    duplicates.cpp
    env.cpp
    journal.cpp
    main.cpp
    misc.cpp
    os.cpp
    page.cpp
    page_manager.cpp
    recordnumber.cpp
    #recovery.cpp
    remote.cpp
    simd.cpp
    txn.cpp
    txn_cursor.cpp
    upscaledb.cpp
    uqi.cpp
    #zint32.cpp
)

target_compile_options( ${TEST_NAME} PRIVATE ${ROMZ_CXX_FLAGS} )

target_include_directories( ${TEST_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries( ${TEST_NAME} 
    -fprofile-arcs 
    ups-5upscaledb
    ups-4db
    ups-4env
    ups-4txn
    ups-4cursor
    ups-4uqi
    ups-3page_manager
    ups-3journal
    ups-3changeset
    ups-3btree
    ups-3blob_manager
    ups-2config
    ups-2page
    ups-2compressor
    ups-2device
    ups-1os
    ups-1mem
    ups-1globals
    ups-1errorinducer
    ups-1base
    MurmurHash3
    for
    lzf
    vbyte
    -lboost_system
    -lboost_thread
    -lboost_filesystem
    -lpthread 
    -ldl
)

add_test( NAME ${TEST_NAME} COMMAND ${TEST_NAME} )

