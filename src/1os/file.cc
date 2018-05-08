#include "0root/root.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

#include "file.h"
#include "1errorinducer/errorinducer.h"
#include "1os/os.h"
#include "2config/env_config.h"

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

namespace upscaledb {

#if 0
#  define os_log(x)      ups_log(x)
#else
#  define os_log(x)
#endif

//
// Constructor: creates an empty File handle
//
File::File()
    : m_fd( UPS_INVALID_FD )
    , m_posix_advice( 0 )
{
}

//
// Move constructor: moves ownership of the file handle
//
File::File( File&& other )
    : m_fd( other.m_fd )
    , m_posix_advice( other.m_posix_advice )
{
    other.m_fd = UPS_INVALID_FD;
}

//
// Move assignment operator: moves ownership of the file handle
//
File& File::operator=( File&& other )
{
    m_fd = other.m_fd;
    other.m_fd = UPS_INVALID_FD;
    return *this;
}


//
// Destructor: closes the file
//
File::~File()
{
    close();
}

//
// Returns true if the file is open
//
bool File::is_open() const
{
    return m_fd != UPS_INVALID_FD;
}

//
//
//
void File::lock_exclusive( int fd, bool lock )
{
    int flags;

    if( lock )
        flags = LOCK_EX | LOCK_NB;
    else
        flags = LOCK_UN;

    if( 0 != ::flock( fd, flags ) )
    {
        ups_log(("flock failed with status %u (%s)", errno, strerror(errno)));

        // it seems that linux does not only return EWOULDBLOCK, as stated
        // in the documentation (flock(2)), but also other errors...
        if( errno && lock )
        {
            throw Exception( UPS_WOULD_BLOCK );
        }
        throw Exception( UPS_IO_ERROR );
    }
}

//
//
//
void File::os_read(int fd, uint8_t *buffer, size_t len)
{
    os_log(("os_read: fd=%d, size=%lld", fd, len));

    size_t total = 0;

    while( total < len )
    {
        const int r = read( fd, &buffer[total], len - total );
        if( r < 0 )
        {
            ups_log(("os_read failed with status %u (%s)", errno, strerror(errno)));
            throw Exception( UPS_IO_ERROR );
        }

        if( r == 0 )
        {
            break;
        }

        total += r;
    }

    if( total != len )
    {
        ups_log(("os_read() failed with short read (%s)", strerror(errno)));
        throw Exception( UPS_IO_ERROR );
    }
}

//
//
//
void File::os_write( int fd, const void *buffer, size_t len )
{
    size_t total = 0;
    const char *p = (const char *)buffer;

    while( total < len )
    {
        const int w = ::write( fd, p + total, len - total );
        if( w < 0 )
        {
            ups_log(("os_write failed with status %u (%s)", errno, strerror(errno)));
            throw Exception(UPS_IO_ERROR);
        }

        if( w == 0 )
        {
            break;
        }

        total += w;
    }

    if( total != len )
    {
        ups_log(("os_write() failed with short read (%s)", strerror(errno)));
        throw Exception(UPS_IO_ERROR);
    }
}


//
// Get the page allocation granularity of the operating system
//
size_t File::granularity()
{
    return (size_t)sysconf(_SC_PAGE_SIZE);
}

//
// Sets the parameter for posix_fadvise()
//
void File::set_posix_advice(int advice)
{
    m_posix_advice = advice;
    assert(m_fd != UPS_INVALID_FD);


    if( m_posix_advice == EnvConfig::UPS_POSIX_FADVICE_RANDOM )
    {
        const int r = ::posix_fadvise( m_fd, 0, 0, POSIX_FADV_RANDOM );
        if( r != 0 )
        {
            ups_log(("posix_fadvise failed with status %d (%s)", errno, strerror(errno)));
            throw Exception( UPS_IO_ERROR );
        }
    }

}

//
// Maps a file in memory
//
// mmap is called with MAP_PRIVATE - the allocated buffer
// is just a copy of the file; writing to the buffer will not alter
// the file itself.
//
void File::mmap(uint64_t position, size_t size, bool readonly, uint8_t **buffer) const
{
    os_log(("File::mmap: fd=%d, position=%lld, size=%lld", m_fd, position, size));

    UPS_INDUCE_ERROR( ErrorInducer::kFileMmap );

    int prot = PROT_READ;
    if( !readonly )
        prot |= PROT_WRITE;


    *buffer = (uint8_t *)::mmap( 0, size, prot, MAP_PRIVATE, m_fd, position );
    if( *buffer == MAP_FAILED )
    {
        *buffer = nullptr;
        ups_log(("mmap failed with status %d (%s)", errno, strerror(errno)));
        throw Exception( UPS_IO_ERROR );
    }

    if( m_posix_advice == EnvConfig::UPS_POSIX_FADVICE_RANDOM )
    {
        const int r = ::madvise( *buffer, size, MADV_RANDOM );
        if( r != 0 )
        {
            ups_log(("madvise failed with status %d (%s)", errno, strerror(errno)));
            throw Exception( UPS_IO_ERROR );
        }
    }

}

//
// Unmaps a buffer
//
void File::munmap(void *buffer, size_t size) const
{
    os_log(("File::munmap: size=%lld", size));

    const int r = ::munmap(buffer, size);
    if( r )
    {
        ups_log(("munmap failed with status %d (%s)", errno, strerror(errno)));
        throw Exception( UPS_IO_ERROR );
    }

}

//
// Positional read from a file
//
void File::pread(uint64_t addr, void *buffer, size_t len) const
{
    os_log(("File::pread: fd=%d, address=%lld, size=%lld", m_fd, addr, len));

    size_t total = 0;

    while( total < len )
    {
        const ssize_t r = ::pread( m_fd, (uint8_t *)buffer + total, len - total, addr + total );
        if( r < 0 )
        {
            ups_log(("File::pread failed with status %u (%s)", errno, strerror(errno)));
            throw Exception( UPS_IO_ERROR );
        }

        if( r == 0 )
        {
            break;
        }

        total += r;
    }

    if( total != len )
    {
        ups_log(("File::pread() failed with short read (%s)", strerror(errno)));
        throw Exception( UPS_IO_ERROR );
    }
}

//
// Positional write to a file
//
void File::pwrite( uint64_t addr, const void *buffer, size_t len ) const
{
    os_log(("File::pwrite: fd=%d, address=%lld, size=%lld", m_fd, addr, len));

    size_t total = 0;

    while( total < len )
    {
        const ssize_t s = ::pwrite( m_fd, buffer, len, addr + total );
        if (s < 0)
        {
            ups_log(("pwrite() failed with status %u (%s)", errno, strerror(errno)));
            throw Exception( UPS_IO_ERROR );
        }

        if( s == 0 )
        {
            break;
        }

        total += s;
    }

    if( total != len )
    {
        ups_log(("pwrite() failed with short read (%s)", strerror(errno)));
        throw Exception( UPS_IO_ERROR );
    }
}

//
// Write data to a file; uses the current file position
//
void File::write( const void *buffer, size_t len ) const
{
    os_log(("File::write: fd=%d, size=%lld", m_fd, len));

    os_write( m_fd, buffer, len );
}

//
// Seek position in a file
//
void File::seek( uint64_t offset, int whence ) const
{
    os_log(("File::seek: fd=%d, offset=%lld, whence=%d", m_fd, offset, whence));

    if( ::lseek( m_fd, offset, whence ) == static_cast< off_t >( -1 ) )
    {
        throw Exception( UPS_IO_ERROR );
    }
}

//
// Tell the position in a file
//
uint64_t File::tell() const
{
    const off_t offset = ::lseek( m_fd, 0, SEEK_CUR );

    os_log(("File::tell: fd=%d, offset=%lld", m_fd, offset));

    if ( offset == static_cast< off_t >( -1 ) )
    {
        throw Exception( UPS_IO_ERROR );
    }

    return static_cast< uint64_t >( offset );
}

//
// Returns the size of the file
//
uint64_t File::file_size() const
{
    struct stat st;
    if( ::fstat( m_fd, &st ) == -1 )
    {
        throw Exception( UPS_IO_ERROR );
    }
    const uint64_t size = st.st_size;

    os_log(("File::file_size: fd=%d, size=%lld", m_fd, size));

    return size;
}

//
// Truncate/resize the file
//
void File::truncate( uint64_t newsize ) const
{
    os_log(("File::truncate: fd=%d, size=%lld", m_fd, newsize));

    if( ::ftruncate( m_fd, newsize ) )
    {
        throw Exception( UPS_IO_ERROR );
    }
}

//
// Creates a new file
//
void File::create(const char *filename, uint32_t mode)
{
    const int osflags = O_CREAT | O_RDWR | O_TRUNC | O_NOATIME ;

    const int fd = ::open( filename, osflags, mode ? mode : 0644 );
    if( fd < 0 )
    {
        ups_log(("creating file %s failed with status %u (%s)", filename, errno, strerror(errno)));
        throw Exception( UPS_IO_ERROR );
    }

    /* lock the file - this is default behaviour since 1.1.0 */
    lock_exclusive( fd, true );

    m_fd = fd;
}

//
// Flushes a file
//
void File::flush() const
{
    os_log(("File::flush: fd=%d", m_fd));
    /* unlike fsync(), fdatasync() does not flush the metadata unless
   * it's really required. it's therefore a lot faster. */
#if HAVE_FDATASYNC && !__APPLE__
    if (fdatasync(m_fd) == -1) {
#else
    if (fsync(m_fd) == -1) {
#endif
        ups_log(("fdatasync failed with status %u (%s)", errno, strerror(errno)));
        throw Exception( UPS_IO_ERROR );
    }
}

//
// Opens an existing file
//
void File::open(const char *filename, bool read_only)
{
    int osflags = O_NOATIME;

    if( read_only )
        osflags |= O_RDONLY;
    else
        osflags |= O_RDWR;


    const int fd = ::open( filename, osflags );
    if( fd < 0 )
    {
        ups_log(("opening file %s failed with status %u (%s)", filename, errno, strerror(errno)));
        throw Exception( errno == ENOENT ? UPS_FILE_NOT_FOUND : UPS_IO_ERROR );
    }

    /* lock the file - this is default behaviour since 1.1.0 */
    lock_exclusive( fd, true );

    m_fd = fd;
}

//
// Closes the file descriptor
//
void File::close()
{
    if( m_fd == UPS_INVALID_FD )
        return;

    // on posix, we most likely don't want to close descriptors 0 and 1
    assert( m_fd != 0 && m_fd != 1 );

    // unlock the file - this is default behaviour since 1.1.0
    lock_exclusive( m_fd, false );

    // now close the descriptor
    if( ::close( m_fd ) == -1 )
    {
        throw Exception( UPS_IO_ERROR );
    }

    m_fd = UPS_INVALID_FD;
}


}
