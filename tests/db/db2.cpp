#include "gtest/gtest.h"
#include <ups/upscaledb.h>

void create_db( const char* path, uint16_t db_name )
{
    ups_status_t st;
    ups_env_t *env;
    ups_db_t *db;

    // parameters for ups_env_create_db
    ups_parameter_t params[] =
    {
        { UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32 },
        { UPS_PARAM_RECORD_SIZE, sizeof( uint32_t ) },
        { 0, 0 }
    };

    // First create a new upscaledb Environment
    st = ups_env_create( &env, path, 0, 0664, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // And in this Environment we create a new Database for uint32-keys and uint32-records.
    st = ups_env_create_db( env, &db, db_name, 0, params );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // Insert some key/value pairs
    for( uint32_t i = 0; i < 20; i++ )
    {
        ups_key_t key;
        ups_record_t rec;

        key.data = &i;
        key.size = sizeof( i );

        rec.size = key.size;
        rec.data = key.data;

        st = ups_db_insert( db, 0, &key, &rec, 0 );
        ASSERT_TRUE( st == UPS_SUCCESS );
    }

    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    ASSERT_TRUE( st == UPS_SUCCESS );
}

void copy_db( ups_db_t *source, ups_db_t *dest )
{
    ups_cursor_t *cursor;  /* upscaledb cursor object */
    ups_status_t st;
    ups_key_t key;
    ups_record_t rec;

    // create a new cursor
    st = ups_cursor_create( &cursor, source, 0, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // get a cursor to the source database
    st = ups_cursor_move( cursor, &key, &rec, UPS_CURSOR_FIRST );
    ASSERT_TRUE( st == UPS_SUCCESS );

    while( true )
    {
        // insert this element into the new database
        st = ups_db_insert( dest, 0, &key, &rec, UPS_DUPLICATE );
        ASSERT_TRUE( st == UPS_SUCCESS );

        // fetch the next item, and repeat till we've reached the end of the database
        st = ups_cursor_move( cursor, &key, &rec, UPS_CURSOR_NEXT) ;
        if( st == UPS_KEY_NOT_FOUND )
            break;

        ASSERT_TRUE( st == UPS_SUCCESS );

    }

    // clean up and return
    st = ups_cursor_close(cursor);
    ASSERT_TRUE( st == UPS_SUCCESS );
}


TEST( db, copying )
{
    ups_status_t st;
    ups_env_t *env = nullptr;
    ups_db_t *src_db = nullptr;
    ups_db_t *dest_db = nullptr;
    const uint16_t src_name = 1;
    const uint16_t dest_name = 2;
    const char env_path[] = "test.db";

    create_db( env_path, src_name );

    // open the Environment
    st = ups_env_open( &env, env_path, 0, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // open the source database
    st = ups_env_open_db( env, &src_db, src_name, 0, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // create the destination database
    st = ups_env_create_db( env, &dest_db, dest_name, UPS_ENABLE_DUPLICATE_KEYS, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // copy the data
    copy_db( src_db, dest_db );

    // clean up and return
    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    ASSERT_TRUE( st == UPS_SUCCESS );
}

