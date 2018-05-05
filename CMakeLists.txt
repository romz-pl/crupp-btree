cmake_minimum_required (VERSION 3.0)
project (crupp-btree)

#SET( ROMZ_CXX_FLAGS -Wall -Wpedantic -Wextra -O2 -std=c++11 )
SET( ROMZ_CXX_FLAGS -Wall -Wpedantic -Wextra -pthread -g -O0 -fprofile-arcs -ftest-coverage -std=c++11 )

include(CTest)
enable_testing(true)
add_subdirectory(3rdparty/googletest)
add_subdirectory(3rdparty/murmurhash3)

add_subdirectory(src/1base)
add_subdirectory(src/1globals)
add_subdirectory(src/1errorinducer)
add_subdirectory(src/1mem)
add_subdirectory(src/1os)


#add_subdirectory(tests)

