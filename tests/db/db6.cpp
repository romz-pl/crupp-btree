#include "gtest/gtest.h"
#include <ups/upscaledb.hpp>



TEST( db, cpp_interface )
{
    const uint32_t LOOP = 10;
    const uint16_t db_name = 1;

    uint32_t i;
    upscaledb::env env;          /* upscaledb environment object */
    upscaledb::db db;            /* upscaledb database object */
    upscaledb::key key;          /* a key */
    upscaledb::record record;    /* a record */

    /* parameters for ups_env_create_db */
    ups_parameter_t params[] =
    {
        {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
        {UPS_PARAM_RECORD_SIZE, sizeof(uint32_t)},
        {0, 0}
    };

    /* Create a new environment file and a database in this environment */
    env.create( "test.db" );
    db = env.create_db( db_name, 0, params );

    //
    // Now we can insert, delete or lookup values in the database
    //
    // for our test program, we just insert a few values, then look them
    // up, then delete them and try to look them up again (which will fail).
    //
    for( i = 0; i < LOOP; i++ )
    {
        key.set_size( sizeof( i ) );
        key.set_data( &i );

        record.set_size( sizeof( i ) );
        record.set_data( &i );

        db.insert( &key, &record );
    }

    //
    // Now lookup all values
    //
    // for db::find(), we could use the flag UPS_RECORD_USER_ALLOC, if WE
    // allocate record.data (otherwise the memory is automatically allocated
    // by upscaledb)
    //
    for( i = 0; i < LOOP; i++ )
    {
        key.set_size( sizeof( i ) );
        key.set_data( &i );

        record = db.find( &key );

        // Check if the value is ok
        ASSERT_TRUE( *reinterpret_cast<uint32_t *>( record.get_data() ) == i );
    }

    //
    // Close the database handle, then re-open it (just to demonstrate how to open a database file)
    //
    db.close();
    env.close();
    env.open( "test.db" );
    db = env.open_db( db_name );

    // now erase all values
    for( i = 0; i < LOOP; i++ )
    {
        key.set_size( sizeof( i ) );
        key.set_data( &i );

        db.erase( &key );
    }

    //
    // Once more we try to find all values. Every db::find() call must now fail with UPS_KEY_NOT_FOUND
    //
    for( i = 0; i < LOOP; i++ )
    {
        key.set_size( sizeof( i ) );
        key.set_data( &i );


        ASSERT_ANY_THROW( db.find( &key ) );
    }
}

TEST( db, transaction )
{
    ups_env_t* env;
    ups_env_create( &env, "test.db", UPS_ENABLE_TRANSACTIONS, 0664, 0 );

    ups_parameter_t params[] =
    {
        {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
        {0, 0}
    };

    ups_txn_t* txn;
    ups_txn_begin( &txn, env, 0, 0, 0 );

    ups_db_t* db;
    ups_env_create_db( env, &db, 1, 0, params );

    std::uint32_t rec_no = 4321;
    for( std::uint32_t i = 0; i < rec_no; i++)
    {
        ups_key_t key = ups_make_key( &i, sizeof( i ) );
        ups_record_t record;

        ups_db_insert( db, txn, &key, &record, 0 );
    }

    ups_txn_commit( txn, 0 );

    uint64_t size;
    ups_db_count( db, 0, 0, &size );
    ASSERT_TRUE( size == rec_no );
}
