#include "gtest/gtest.h"
#include <ups/upscaledb.h>


TEST( db, basic_usage )
{
    ups_status_t st;       /* status variable */
    ups_env_t *env;        /* upscaledb environment object */
    ups_db_t *db;          /* upscaledb database object */
    ups_key_t key;         /* the structure for a key */
    ups_record_t record;   /* the structure for a record */

    // parameters for ups_env_create_db
    ups_parameter_t params[] =
    {
        { UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32 },
        { UPS_PARAM_RECORD_SIZE, sizeof( uint32_t ) },
        { 0, 0 }
    };

    const uint32_t LOOP = 10;
    const uint16_t  DATABASE_NAME = 1;

    // First create a new upscaledb Environment
    st = ups_env_create( &env, "test.db", 0, 0664, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // And in this Environment we create a new Database for uint32-keys and uint32-records.
    st = ups_env_create_db( env, &db, DATABASE_NAME, 0, params );
    ASSERT_TRUE( st == UPS_SUCCESS );

    //
    // now we can insert, delete or lookup values in the database
    //
    // for our test program, we just insert a few values, then look them
    // up, then delete them and try to look them up again (which will fail).
    //
    for( uint32_t i = 0; i < LOOP; i++ )
    {
        key.data = &i;
        key.size = sizeof( i );

        record.size = key.size;
        record.data = key.data;

        st = ups_db_insert( db, 0, &key, &record, 0 );
        ASSERT_TRUE( st == UPS_SUCCESS );
    }

    //
    // now lookup all values
    //
    // for ups_db_find(), we could use the flag UPS_RECORD_USER_ALLOC, if WE
    // allocate record.data (otherwise the memory is automatically allocated
    // by upscaledb)
    //
    for( uint32_t i = 0; i < LOOP; i++ )
    {
        key.data = &i;
        key.size = sizeof( i );

        st = ups_db_find( db, 0, &key, &record, 0 );
        ASSERT_TRUE( st == UPS_SUCCESS );

        // check if the value is ok
        ASSERT_TRUE( *(uint32_t *)record.data == i );
    }

    //
    // close the database handle, then re-open it (to demonstrate how to open
    // an Environment and a Database)
    //
    st = ups_db_close( db, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    st = ups_env_close( env, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    st = ups_env_open( &env, "test.db", 0, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    st = ups_env_open_db( env, &db, DATABASE_NAME, 0, 0 );
    ASSERT_TRUE( st == UPS_SUCCESS );

    // now erase all values
    for( uint32_t i = 0; i < LOOP; i++ )
    {
        key.size = sizeof(i);
        key.data = &i;

        st = ups_db_erase( db, 0, &key, 0 );
        ASSERT_TRUE( st == UPS_SUCCESS );
    }

    //
    // once more we try to find all values... every ups_db_find() call must
    // now fail with UPS_KEY_NOT_FOUND
    //
    for( uint32_t i = 0; i < LOOP; i++ )
    {
        key.size = sizeof( i );
        key.data = &i;

        st = ups_db_find( db, 0, &key, &record, 0 );
        ASSERT_TRUE( st == UPS_KEY_NOT_FOUND );
    }

    // we're done! close the handles.
    // UPS_AUTO_CLEANUP will also close the 'db' handle
    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    ASSERT_TRUE( st == UPS_SUCCESS );
}

