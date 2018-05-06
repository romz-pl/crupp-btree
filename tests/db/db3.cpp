#include "gtest/gtest.h"
#include <cstring>
#include <ups/upscaledb.h>
#include <vector>
#include <string>
#include <algorithm>
#include "random_string.h"


static int
my_string_compare(ups_db_t*,
                  const uint8_t *lhs, uint32_t lhs_length,
                  const uint8_t *rhs, uint32_t rhs_length)
{
    const int s = std::strncmp( reinterpret_cast< const char *>( lhs ),
                                reinterpret_cast< const char *>( rhs ),
                                std::min( lhs_length, rhs_length ) );

    if( s < 0 )
        return -1;

    if( s > 0 )
        return +1;

    return 0;
}

TEST( db, sorting )
{
    const uint16_t DATABASE_NAME = 1;

    ups_status_t st;
    ups_env_t *env = nullptr;
    ups_db_t *db = nullptr;
    ups_key_t key;
    ups_record_t record;

    // parameters for ups_env_create_db
    ups_parameter_t params[] =
    {
        {UPS_PARAM_KEY_TYPE, UPS_TYPE_CUSTOM},
        {UPS_PARAM_RECORD_SIZE, 0}, // we do not store records, only keys
        {0, 0}
    };


    // Create a new upscaledb Environment.
    st = ups_env_create( &env, "test.db", 0, 0664, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // Create a new Database in the new Environment.
    // The UPS_TYPE_CUSTOM parameter allows us to set a custom compare function.
    st = ups_env_create_db( env, &db, DATABASE_NAME, UPS_ENABLE_DUPLICATE_KEYS, params );
    ASSERT_TRUE( st == UPS_SUCCESS );


    // Since we use strings as our database keys we use our own comparison
    // function based on strcmp instead of the default memcmp function.
    st = ups_db_set_compare_func( db, my_string_compare );
    ASSERT_TRUE( st == UPS_SUCCESS );

    const std::size_t max_length = 200;
    for( std::uint32_t i = 0; i < 10000; i++ )
    {
        const std::string s = random_string( max_length );

        key.data = const_cast< char* >( s.c_str() );
        key.size = s.size() + 1; // also store the terminating 0-byte
        st = ups_db_insert( db, 0, &key, &record, 0 );
        ASSERT_TRUE( st == UPS_SUCCESS || st == UPS_DUPLICATE_KEY );

    }

    // create a cursor
    ups_cursor_t *cursor = nullptr;
    st = ups_cursor_create( &cursor, db, 0, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    std::vector< std::string > res;

    // iterate over all items with UPS_CURSOR_NEXT
    while( true )
    {
        st = ups_cursor_move( cursor, &key, &record, UPS_CURSOR_NEXT );
        if( st == UPS_KEY_NOT_FOUND )
            break;

        ASSERT_TRUE( st == UPS_SUCCESS );

        const std::string s( reinterpret_cast< const char *>( key.data ) );
        res.push_back( s );
    }

    ASSERT_TRUE( std::is_sorted( res.begin(), res.end() ) );


    // Then close the handles; the flag UPS_AUTO_CLEANUP will automatically close all database
    // and cursors, and we do not need to call ups_cursor_close and ups_db_close
    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    ASSERT_TRUE( st == UPS_SUCCESS );

}

