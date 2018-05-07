set( TEST_NAME db_test )

add_executable( ${TEST_NAME}
    db1.cpp
    db2.cpp
    db3.cpp
    db4.cpp
    db5.cpp
    db6.cpp
    env1.cpp
    env2.cpp
    env3.cpp
    main.cpp
    random_string.cpp
)

target_compile_options( ${TEST_NAME} PRIVATE ${ROMZ_CXX_FLAGS} )

target_include_directories( ${TEST_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries( ${TEST_NAME} 
    -fprofile-arcs 
    gtest 
    gtest_main 
    pthread
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

