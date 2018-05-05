cmake_minimum_required (VERSION 3.0)
project (amittai-btree)

#SET( ROMZ_CXX_FLAGS -Wall -Wpedantic -Wextra -O2 -std=c++11 )
SET( ROMZ_CXX_FLAGS -Wall -Wpedantic -Wextra -pthread -g -O0 -fprofile-arcs -ftest-coverage -std=c++11 )

include(CTest)
enable_testing(true)
add_subdirectory(3rdparty/googletest)

#add_subdirectory(src)
#add_subdirectory(tests)

