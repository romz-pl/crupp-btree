#include "gtest/gtest.h"
#include "2device/device_disk.h"

TEST( DiskDevice, create_open )
{
    upscaledb::EnvConfig config;
    config.filename = "test.db";
    upscaledb::DiskDevice dd( config );

    ASSERT_NO_THROW( dd.create() );
    ASSERT_NO_THROW( dd.is_open() );

    const size_t page_no = 1234;
    const size_t requested_length = config.page_size_bytes;
    const size_t requested_file_size = page_no * requested_length;
    while( dd.file_size() < requested_file_size )
    {
        ASSERT_NO_THROW( dd.alloc( requested_length ) );
    }

    ASSERT_NO_THROW( dd.close() );

    ASSERT_NO_THROW( dd.open() );

    const uint32_t len = config.page_size_bytes;

    for( size_t i = 0; i < page_no; i++ )
    {
        std::vector< char > buf( len, i + 1 );
        std::vector< char > tmp( len, 0 );

        ASSERT_NO_THROW( dd.write( i * len, buf.data(), len ) );
        ASSERT_NO_THROW( dd.read ( i * len, tmp.data(), len ) );

        ASSERT_TRUE( buf == tmp );
        
        upscaledb::Page page( &dd );
        ASSERT_NO_THROW( dd.read_page( &page, offset ) );
        ASSERT_TRUE( std::memcmp( page.data(), tmp.data(), len ) == 0 );
        ASSERT_NO_THROW( dd.free_page( &page ) );
    }

    ASSERT_NO_THROW( dd.close() );
	
}

