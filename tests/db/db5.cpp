#include "gtest/gtest.h"
#include <cstring>
#include <ups/upscaledb.h>
#include <vector>
#include <string>
#include <algorithm>
#include "random_string.h"


TEST( db, duplicates )
{
    const uint16_t DATABASE_NAME = 1;

    ups_status_t st;
    ups_env_t *env = nullptr;
    ups_db_t *db = nullptr;
    ups_cursor_t *cursor = nullptr;

    ups_key_t key;
    ups_record_t record;

    /* we insert 4 byte records only */
    ups_parameter_t params[] =
    {
        {UPS_PARAM_RECORD_SIZE, sizeof(uint32_t)},
        {0, 0}
    };

    // Create a new Database with support for duplicate keys
    st = ups_env_create( &env, 0, UPS_IN_MEMORY, 0664, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    st = ups_env_create_db( env, &db, DATABASE_NAME, UPS_ENABLE_DUPLICATE_KEYS, params );
    ASSERT_TRUE( st == UPS_SUCCESS );

    std::vector< std::string > oryg;

    const std::size_t max_length = 3;
    for( std::uint32_t i = 0; i < 10000; i++ )
    {
        const std::string s = random_string( max_length );

        key.data = const_cast< char* >( s.c_str() );
        key.size = s.size() + 1; // also store the terminating 0-byte

        record.data = &i;
        record.size = sizeof(i);

        st = ups_db_insert( db, 0, &key, &record, UPS_DUPLICATE );
        ASSERT_TRUE( st == UPS_SUCCESS );

        oryg.push_back( s );
    }


    // Create a cursor
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

    std::sort( oryg.begin(), oryg.end() );

    ASSERT_TRUE( res == oryg );

    // Then close the handles; the flag UPS_AUTO_CLEANUP will automatically close all database
    // and cursors, and we do not need to call ups_cursor_close and ups_db_close
    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    ASSERT_TRUE( st == UPS_SUCCESS );
}

