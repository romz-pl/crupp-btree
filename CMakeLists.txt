cmake_minimum_required (VERSION 3.0)
project (crupp-btree)

find_package( Boost REQUIRED )

#SET( ROMZ_CXX_FLAGS -Wall -Wpedantic -Wextra -O2 -std=c++11 )
SET( ROMZ_CXX_FLAGS -Wall -Wpedantic -Wextra -pthread -g -O0 -fprofile-arcs -ftest-coverage -std=c++11 )

SET( ROMZ_C_FLAGS -Wall -Wpedantic -Wextra -pthread -g -O0 -fprofile-arcs -ftest-coverage )


include(CTest)
enable_testing(true)
add_subdirectory(3rdparty/googletest)
add_subdirectory(3rdparty/murmurhash3)
add_subdirectory(3rdparty/liblzf)
add_subdirectory(3rdparty/libfor)
add_subdirectory(3rdparty/libvbyte)


add_subdirectory(src/1base)
add_subdirectory(src/1globals)
add_subdirectory(src/1errorinducer)
add_subdirectory(src/1mem)
add_subdirectory(src/1os)

add_subdirectory(src/2compressor)
add_subdirectory(src/2page)

add_subdirectory(src/3btree)
add_subdirectory(src/3blob_manager)
add_subdirectory(src/3changeset)
add_subdirectory(src/3journal)
add_subdirectory(src/3page_manager)

add_subdirectory(src/4cursor)
add_subdirectory(src/4db)
add_subdirectory(src/4env)
add_subdirectory(src/4txn)
add_subdirectory(src/4uqi)


add_subdirectory(src/5upscaledb)

add_subdirectory(tests)

