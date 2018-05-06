#include "gtest/gtest.h"
#include <ups/upscaledb.h>
#include <vector>
#include <string>
#include <algorithm>
#include "random_string.h"

TEST( db, record_number )
{
    const uint16_t DATABASE_NAME = 1;

    ups_status_t st;
    ups_env_t *env = nullptr;
    ups_db_t *db  = nullptr;
    ups_cursor_t *cursor  = nullptr;
    ups_key_t key;
    ups_record_t record;


    // Create a new upscaledb "record number" Database.
    // We could create an in-memory-Environment to speed up the sorting.
    st = ups_env_create( &env, "test.db", 0, 0664, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    st = ups_env_create_db( env, &db, DATABASE_NAME, UPS_RECORD_NUMBER32, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    std::vector< std::string > oryg;

    const std::size_t max_length = 200;
    for( std::uint32_t i = 0; i < 10000; i++ )
    {
        std::uint32_t recno;

        key.flags = UPS_KEY_USER_ALLOC;
        key.data = &recno;
        key.size = sizeof(recno);

        const std::string s = random_string( max_length );
        oryg.push_back( s );

        record.data = const_cast< char* >( s.c_str() );
        record.size = s.size() + 1; // also store the terminating 0-byte
        st = ups_db_insert( db, 0, &key, &record, 0 );
        ASSERT_TRUE( st == UPS_SUCCESS || st == UPS_DUPLICATE_KEY );

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

        const std::string s( reinterpret_cast< const char *>( record.data ) );
        res.push_back( s );
    }

    ASSERT_TRUE( oryg == res );

    // Then close the handles; the flag UPS_AUTO_CLEANUP will automatically close all database
    // and cursors, and we do not need to call ups_cursor_close and ups_db_close
    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    ASSERT_TRUE( st == UPS_SUCCESS );


}

