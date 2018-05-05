
set( LIB_NAME ups-1os )

add_library( ${LIB_NAME} STATIC
    os.cc
    os_posix.cc
#    os_win32.cc
)

target_include_directories( ${LIB_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/src 
    ${PROJECT_SOURCE_DIR}/include
    )

target_compile_options( ${LIB_NAME} PRIVATE ${ROMZ_CXX_FLAGS} )


