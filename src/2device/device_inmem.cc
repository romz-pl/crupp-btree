#include "device_inmem.h"
#include <assert.h>
#include "1base/error.h"
#include "1mem/mem.h"
#include "2page/page.h"

namespace upscaledb {

//
// constructor
//
InMemoryDevice::InMemoryDevice(const EnvConfig &config)
    : Device(config)
{
    is_open_ = false;
    allocated_size_ = 0;
}

//
// Create a new device
//
void InMemoryDevice::create()
{
    is_open_ = true;
}

//
// opens an existing device
//
void InMemoryDevice::open()
{
    assert( !"can't open an in-memory-device" );
    throw Exception( UPS_NOT_IMPLEMENTED );
}

//
// returns true if the device is open
//
bool InMemoryDevice::is_open()
{
    return is_open_;
}

//
// closes the device
//
void InMemoryDevice::close()
{
    assert( is_open_ );
    is_open_ = false;
}

//
// flushes the device
void InMemoryDevice::flush()
{
}

//
// truncate/resize the device
//
void InMemoryDevice::truncate( uint64_t )
{
}

//
// get the current file/storage size
//
uint64_t InMemoryDevice::file_size()
{
    assert(!"this operation is not possible for in-memory-databases");
    throw Exception( UPS_NOT_IMPLEMENTED );
}

//
// seek position in a file
//
void InMemoryDevice::seek( uint64_t, int )
{
    assert( !"can't seek in an in-memory-device" );
    throw Exception( UPS_NOT_IMPLEMENTED );
}

//
// tell the position in a file
//
uint64_t InMemoryDevice::tell()
{
    assert( !"can't tell in an in-memory-device" );
    throw Exception( UPS_NOT_IMPLEMENTED );
}

//
// reads from the device; this function does not use mmap
//
void InMemoryDevice::read( uint64_t, void*, size_t )
{
    assert( !"operation is not possible for in-memory-databases" );
    throw Exception( UPS_NOT_IMPLEMENTED );
}

//
// writes to the device
//
void InMemoryDevice::write( uint64_t, void*, size_t )
{
}

//
// reads a page from the device
//
void InMemoryDevice::read_page( Page*, uint64_t )
{
    assert( !"operation is not possible for in-memory-databases" );
    throw Exception( UPS_NOT_IMPLEMENTED );
}

//
// allocate storage from this device; this function will *NOT* use mmap.
//
uint64_t InMemoryDevice::alloc( size_t size )
{
    if( allocated_size_ + size > config.file_size_limit_bytes )
    {
        throw Exception( UPS_LIMITS_REACHED );
    }

    uint64_t retval = ( uint64_t )Memory::allocate< uint8_t >( size );
    allocated_size_ += size;
    return retval;
}

//
// allocate storage for a page from this device
//
void InMemoryDevice::alloc_page( Page *page )
{
    size_t page_size = config.page_size_bytes;
    if( allocated_size_ + page_size > config.file_size_limit_bytes )
    {
        throw Exception( UPS_LIMITS_REACHED );
    }

    uint8_t *p = Memory::allocate< uint8_t >( page_size );
    page->assign_allocated_buffer( p, ( uint64_t )p );

    allocated_size_ += page_size;
}

//
// frees a page on the device; plays counterpoint to @ref alloc_page
//
void InMemoryDevice::free_page( Page *page )
{
    page->free_buffer();

    assert( allocated_size_ >= config.page_size_bytes );
    allocated_size_ -= config.page_size_bytes;
}

//
// Returns true if the specified range is in mapped memory
//
bool InMemoryDevice::is_mapped( uint64_t, size_t ) const
{
    return false;
}

//
// Removes unused space at the end of the file
//
void InMemoryDevice::reclaim_space()
{
}

//
// releases a chunk of memory previously allocated with alloc()
//
void InMemoryDevice::release( void *ptr, size_t size )
{
    Memory::release( ptr );
    assert( allocated_size_ >= size );
    allocated_size_ -= size;
}


}
