#include "gtest/gtest.h"
#include "2device/device_disk.h"

TEST( DiskDevice, create_open )
{
	upscaledb::EnvConfig config;
	upscaledb::DiskDevice dd( config );
	
	dd.create();
	
}

