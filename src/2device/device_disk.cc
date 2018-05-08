#include "device_disk.h"
#include "1mem/mem.h"
#include "2page/page.h"

namespace upscaledb {

//
//
//
DiskDevice::DiskDevice( const EnvConfig &config )
    : Device( config )
{
    State state;
    state.mmapptr = 0;
    state.mapped_size = 0;
    state.file_size = 0;
    state.excess_at_end = 0;
    swap( m_state, state );
}

//
// Create a new device
//
void DiskDevice::create()
{
    ScopedSpinlock lock( m_mutex );

    File file;
    file.create( config.filename.c_str(), config.file_mode );
    file.set_posix_advice( config.posix_advice );
    m_state.file = std::move( file );
}

//
// opens an existing device
//
// tries to map the file; if it fails then continue with read/write
//
void DiskDevice::open()
{
    bool read_only = ( config.flags & UPS_READ_ONLY ) != 0;

    ScopedSpinlock lock( m_mutex );

    State state = std::move( m_state );
    state.file.open (config.filename.c_str(), read_only );
    state.file.set_posix_advice( config.posix_advice );

    // the file size which backs the mapped ptr
    state.file_size = state.file.file_size();

    if( IS_SET( config.flags, UPS_DISABLE_MMAP ) )
    {
        swap( m_state, state );
        return;
    }

    // make sure we do not exceed the "real" size of the file, otherwise
    // we crash when accessing memory which exceeds the mapping (at least on Win32)
    const size_t granularity = File::granularity();
    if( state.file_size == 0 || state.file_size % granularity )
    {
        swap( m_state, state );
        return;
    }

    state.mapped_size = state.file_size;
    try
    {
        state.file.mmap( 0, state.mapped_size, read_only, &state.mmapptr );
    }
    catch( Exception &ex )
    {
        ups_log(("mmap failed with error %d, falling back to read/write", ex.code));
    }
    swap( m_state, state );
}

//
// returns true if the device is open
//
bool DiskDevice::is_open() const
{
    ScopedSpinlock lock( m_mutex );
    return m_state.file.is_open();
}

//
// closes the device
//
void DiskDevice::close()
{
    ScopedSpinlock lock( m_mutex );
    State state = std::move( m_state );
    if( state.mmapptr )
    {
        state.file.munmap( state.mmapptr, state.mapped_size );
    }
    state.file.close();

    swap( m_state, state );
}

//
// flushes the device
//
void DiskDevice::flush() const
{
    ScopedSpinlock lock( m_mutex );
    m_state.file.flush();
}

//
// truncate/resize the device
//
void DiskDevice::truncate( uint64_t new_file_size )
{
    ScopedSpinlock lock( m_mutex );
    truncate_nolock( new_file_size );
}

//
// get the current file/storage size
//
uint64_t DiskDevice::file_size() const
{
    ScopedSpinlock lock( m_mutex );
    assert( m_state.file_size == m_state.file.file_size() );
    return m_state.file_size;
}

//
// seek to a position in a file
//
void DiskDevice::seek( uint64_t offset, int whence ) const
{
    ScopedSpinlock lock( m_mutex );
    m_state.file.seek( offset, whence );
}

//
// tell the position in a file
//
uint64_t DiskDevice::tell() const
{
    ScopedSpinlock lock( m_mutex );
    return m_state.file.tell();
}

//
// reads from the device; this function does NOT use mmap
//
void DiskDevice::read( uint64_t offset, void *buffer, size_t len ) const
{
    ScopedSpinlock lock( m_mutex );
    m_state.file.pread( offset, buffer, len );

#ifdef UPS_ENABLE_ENCRYPTION
    if (config.is_encryption_enabled)
    {
        AesCipher aes( config.encryption_key, offset );
        aes.decrypt( (uint8_t *)buffer, (uint8_t *)buffer, len );
    }
#endif
}

//
// writes to the device; this function does not use mmap,
// and is responsible for writing the data is run through the file
// filters
//
void DiskDevice::write(uint64_t offset, void *buffer, size_t len) const
{
    ScopedSpinlock lock( m_mutex );

#ifdef UPS_ENABLE_ENCRYPTION
    if (config.is_encryption_enabled)
    {
        // encryption disables direct I/O -> only full pages are allowed
        assert(offset % len == 0);

        uint8_t *encryption_buffer = (uint8_t *)::alloca(len);
        AesCipher aes(config.encryption_key, offset);
        aes.encrypt((uint8_t *)buffer, encryption_buffer, len);
        m_state.file.pwrite(offset, encryption_buffer, len);
        return;
    }
#endif
    m_state.file.pwrite( offset, buffer, len );
}

//
// allocate storage from this device; this function
// will *NOT* return mmapped memory
//
uint64_t DiskDevice::alloc( size_t requested_length )
{
    ScopedSpinlock lock( m_mutex );
    uint64_t address;

    if( m_state.excess_at_end >= requested_length )
    {
        address = m_state.file_size - m_state.excess_at_end;
        m_state.excess_at_end -= requested_length;
    }
    else
    {
        uint64_t excess = 0;
        bool allocate_excess = true;

        // If the file is large enough then allocate more space to avoid
        // frequent calls to ftruncate(); these calls cause bad performance
        // spikes.
        //
        // Disabled on win32 because truncating a mapped file is not allowed!

        if( allocate_excess )
        {
            if( m_state.file_size < requested_length * 100 )
            {
                excess = 0;
            }
            else if( m_state.file_size < requested_length * 250 )
            {
                excess = requested_length * 100;
            }
            else if( m_state.file_size < requested_length * 1000 )
            {
                excess = requested_length * 250;
            }
            else
            {
                excess = requested_length * 1000;
            }
        }

        address = m_state.file_size;
        truncate_nolock( address + requested_length + excess );
        m_state.excess_at_end = excess;
    }
    return address;
}

//
// reads a page from the device; this function CAN return a
// pointer to mmapped memory
//
void DiskDevice::read_page( Page *page, uint64_t address ) const
{
    ScopedSpinlock lock( m_mutex );

    // if this page is in the mapped area: return a pointer into that area.
    // otherwise fall back to read/write.
    if( address < m_state.mapped_size && m_state.mmapptr != 0 )
    {
        // the following line will not throw a C++ exception, but can
        // raise a signal. If that's the case then we don't catch it because
        // something is seriously wrong and proper recovery is not possible.
        page->assign_mapped_buffer( &m_state.mmapptr[ address ], address );
        return;
    }

    // this page is not in the mapped area; allocate a buffer
    if( page->data() == 0 )
    {
        // note that |p| will not leak if file.pread() throws; |p| is stored
        // in the |page| object and will be cleaned up by the caller in
        // case of an exception.
        uint8_t *p = Memory::allocate< uint8_t >( config.page_size_bytes );
        page->assign_allocated_buffer(p, address);
    }

    m_state.file.pread( address, page->data(), config.page_size_bytes );

#ifdef UPS_ENABLE_ENCRYPTION
    if (config.is_encryption_enabled)
    {
        AesCipher aes(config.encryption_key, page->address());
        aes.decrypt((uint8_t *)page->data(), (uint8_t *)page->data(),
                    config.page_size_bytes);
    }
#endif
}

//
// Allocates storage for a page from this device; this function
// will *NOT* return mmapped memory
//
void DiskDevice::alloc_page( Page *page )
{
    uint64_t address = alloc( config.page_size_bytes );
    page->set_address( address );

    // allocate a memory buffer
    uint8_t *p = Memory::allocate< uint8_t >( config.page_size_bytes );
    page->assign_allocated_buffer( p, address );
}
//
// Frees a page on the device; plays counterpoint to |alloc_page|
//
void DiskDevice::free_page( Page *page )
{
    ScopedSpinlock lock( m_mutex );
    assert( page->data() != 0 );
    page->free_buffer();
}

//
// Returns true if the specified range is in mapped memory
//
bool DiskDevice::is_mapped( uint64_t file_offset, size_t size ) const
{
    return file_offset + size <= m_state.mapped_size;
}

//
// Removes unused space at the end of the file
//
void DiskDevice::reclaim_space()
{
    ScopedSpinlock lock( m_mutex );
    if (m_state.excess_at_end > 0)
    {
        truncate_nolock( m_state.file_size - m_state.excess_at_end );
        m_state.excess_at_end = 0;
    }
}

//
// Returns a pointer directly into mapped memory
//
uint8_t* DiskDevice::mapped_pointer( uint64_t address ) const
{
    return &m_state.mmapptr[ address ];
}



//
// truncate/resize the device, sans locking
//
void DiskDevice::truncate_nolock( uint64_t new_file_size )
{
    if( new_file_size > config.file_size_limit_bytes )
    {
        throw Exception( UPS_LIMITS_REACHED );
    }

    m_state.file.truncate( new_file_size );
    m_state.file_size = new_file_size;
}

}

