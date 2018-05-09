#include "gtest/gtest.h"
#include "2device/device_disk.h"

TEST( DiskDevice, create_open )
{
	upscaledb::EnvConfig config;
	config.filename = "test.db";
	
	upscaledb::DiskDevice dd( config );
	
	ASSERT_NO_THROW( dd.create() );
	
}

