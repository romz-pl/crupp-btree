#include "0root/root.h"

#if HAVE_MMAP
#  include <sys/mman.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

#include "file.h"
#include "1errorinducer/errorinducer.h"

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
//
//
void File::enable_largefile(int fd)
{
  // not available on cygwin...
#ifdef HAVE_O_LARGEFILE
  int oflag = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, oflag | O_LARGEFILE);
#endif
}

//
//
//
void File::lock_exclusive(int fd, bool lock)
{
#ifdef UPS_SOLARIS
  // SunOS 5.9 doesn't have LOCK_* unless i include /usr/ucbinclude; but then,
  // mmap behaves strangely (the first write-access to the mmapped buffer
  // leads to a segmentation fault).
  //
  // Tell me if this troubles you/if you have suggestions for fixes.
#else
  int flags;

  if (lock)
    flags = LOCK_EX | LOCK_NB;
  else
    flags = LOCK_UN;

  if (0 != flock(fd, flags)) {
    ups_log(("flock failed with status %u (%s)", errno, strerror(errno)));
    // it seems that linux does not only return EWOULDBLOCK, as stated
    // in the documentation (flock(2)), but also other errors...
    if (errno && lock)
      throw Exception(UPS_WOULD_BLOCK);
    throw Exception(UPS_IO_ERROR);
  }
#endif
}

//
//
//
void File::os_read(ups_fd_t fd, uint8_t *buffer, size_t len)
{
  os_log(("os_read: fd=%d, size=%lld", fd, len));

  int r;
  size_t total = 0;

  while (total < len) {
    r = read(fd, &buffer[total], len - total);
    if (r < 0) {
      ups_log(("os_read failed with status %u (%s)", errno, strerror(errno)));
      throw Exception(UPS_IO_ERROR);
    }
    if (r == 0)
      break;
    total += r;
  }

  if (total != len) {
    ups_log(("os_read() failed with short read (%s)", strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }
}

//
//
//
void File::os_write(ups_fd_t fd, const void *buffer, size_t len)
{
  int w;
  size_t total = 0;
  const char *p = (const char *)buffer;

  while (total < len) {
    w = ::write(fd, p + total, len - total);
    if (w < 0) {
      ups_log(("os_write failed with status %u (%s)", errno,
                              strerror(errno)));
      throw Exception(UPS_IO_ERROR);
    }
    if (w == 0)
      break;
    total += w;
  }

  if (total != len) {
    ups_log(("os_write() failed with short read (%s)", strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }
}


//
//
//
size_t File::granularity()
{
  return (size_t)sysconf(_SC_PAGE_SIZE);
}

//
//
//
void File::set_posix_advice(int advice)
{
  m_posix_advice = advice;
  assert(m_fd != UPS_INVALID_FD);

#if HAVE_POSIX_FADVISE
  if (m_posix_advice == UPS_POSIX_FADVICE_RANDOM) {
    int r = ::posix_fadvise(m_fd, 0, 0, POSIX_FADV_RANDOM);
    if (r != 0) {
      ups_log(("posix_fadvise failed with status %d (%s)",
                              errno, strerror(errno)));
      throw Exception(UPS_IO_ERROR);
    }
  }
#endif
}

//
//
//
void File::mmap(uint64_t position, size_t size, bool readonly, uint8_t **buffer)
{
  os_log(("File::mmap: fd=%d, position=%lld, size=%lld", m_fd, position, size));

  UPS_INDUCE_ERROR(ErrorInducer::kFileMmap);

  int prot = PROT_READ;
  if (!readonly)
    prot |= PROT_WRITE;

#if HAVE_MMAP
  *buffer = (uint8_t *)::mmap(0, size, prot, MAP_PRIVATE, m_fd, position);
  if (*buffer == (void *)-1) {
    *buffer = 0;
    ups_log(("mmap failed with status %d (%s)", errno, strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }
#else
  throw Exception(UPS_NOT_IMPLEMENTED);
#endif

#if HAVE_MADVISE
  if (m_posix_advice == UPS_POSIX_FADVICE_RANDOM) {
    int r = ::madvise(*buffer, size, MADV_RANDOM);
    if (r != 0) {
      ups_log(("madvise failed with status %d (%s)", errno, strerror(errno)));
      throw Exception(UPS_IO_ERROR);
    }
  }
#endif
}

//
//
//
void File::munmap(void *buffer, size_t size)
{
  os_log(("File::munmap: size=%lld", size));

#if HAVE_MUNMAP
  int r = ::munmap(buffer, size);
  if (r) {
    ups_log(("munmap failed with status %d (%s)", errno, strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }
#else
  throw Exception(UPS_NOT_IMPLEMENTED);
#endif
}

//
//
//
void File::pread(uint64_t addr, void *buffer, size_t len)
{
  os_log(("File::pread: fd=%d, address=%lld, size=%lld", m_fd, addr, len));

#if HAVE_PREAD
  int r;
  size_t total = 0;

  while (total < len) {
    r = ::pread(m_fd, (uint8_t *)buffer + total, len - total,
                    addr + total);
    if (r < 0) {
      ups_log(("File::pread failed with status %u (%s)", errno,
                              strerror(errno)));
      throw Exception(UPS_IO_ERROR);
    }
    if (r == 0)
      break;
    total += r;
  }

  if (total != len) {
    ups_log(("File::pread() failed with short read (%s)", strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }
#else
  File::seek(addr, kSeekSet);
  os_read(m_fd, (uint8_t *)buffer, len);
#endif
}

//
//
//
void File::pwrite(uint64_t addr, const void *buffer, size_t len)
{
  os_log(("File::pwrite: fd=%d, address=%lld, size=%lld", m_fd, addr, len));

#if HAVE_PWRITE
  ssize_t s;
  size_t total = 0;

  while (total < len) {
    s = ::pwrite(m_fd, buffer, len, addr + total);
    if (s < 0) {
      ups_log(("pwrite() failed with status %u (%s)", errno, strerror(errno)));
      throw Exception(UPS_IO_ERROR);
    }
    if (s == 0)
      break;
    total += s;
  }

  if (total != len) {
    ups_log(("pwrite() failed with short read (%s)", strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }
#else
  seek(addr, kSeekSet);
  write(buffer, len);
#endif
}

//
//
//
void File::write(const void *buffer, size_t len)
{
  os_log(("File::write: fd=%d, size=%lld", m_fd, len));
  os_write(m_fd, buffer, len);
}

//
//
//
void File::seek(uint64_t offset, int whence) const
{
  os_log(("File::seek: fd=%d, offset=%lld, whence=%d", m_fd, offset, whence));
  if (lseek(m_fd, offset, whence) < 0)
    throw Exception(UPS_IO_ERROR);
}

//
//
//
uint64_t File::tell() const
{
  uint64_t offset = lseek(m_fd, 0, SEEK_CUR);
  os_log(("File::tell: fd=%d, offset=%lld", m_fd, offset));
  if (offset == (uint64_t) - 1)
    throw Exception(UPS_IO_ERROR);
  return offset;
}

//
//
//
uint64_t File::file_size() const
{
  seek(0, kSeekEnd);
  uint64_t size = tell();
  os_log(("File::file_size: fd=%d, size=%lld", m_fd, size));
  return size;
}

//
//
//
void File::truncate(uint64_t newsize)
{
  os_log(("File::truncate: fd=%d, size=%lld", m_fd, newsize));
  if (ftruncate(m_fd, newsize))
    throw Exception(UPS_IO_ERROR);
}

//
//
//
void File::create(const char *filename, uint32_t mode)
{
  int osflags = O_CREAT | O_RDWR | O_TRUNC;
#if HAVE_O_NOATIME
  osflags |= O_NOATIME;
#endif

  ups_fd_t fd = ::open(filename, osflags, mode ? mode : 0644);
  if (fd < 0) {
    ups_log(("creating file %s failed with status %u (%s)", filename,
        errno, strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }

  /* lock the file - this is default behaviour since 1.1.0 */
  lock_exclusive(fd, true);

  /* enable O_LARGEFILE support */
  enable_largefile(fd);

  m_fd = fd;
}

//
//
//
void File::flush()
{
  os_log(("File::flush: fd=%d", m_fd));
  /* unlike fsync(), fdatasync() does not flush the metadata unless
   * it's really required. it's therefore a lot faster. */
#if HAVE_FDATASYNC && !__APPLE__
  if (fdatasync(m_fd) == -1) {
#else
  if (fsync(m_fd) == -1) {
#endif
    ups_log(("fdatasync failed with status %u (%s)",
        errno, strerror(errno)));
    throw Exception(UPS_IO_ERROR);
  }
}

//
//
//
void File::open(const char *filename, bool read_only)
{
  int osflags = 0;

  if (read_only)
    osflags |= O_RDONLY;
  else
    osflags |= O_RDWR;
#if HAVE_O_NOATIME
  osflags |= O_NOATIME;
#endif

  ups_fd_t fd = ::open(filename, osflags);
  if (fd < 0) {
    ups_log(("opening file %s failed with status %u (%s)", filename,
        errno, strerror(errno)));
    throw Exception(errno == ENOENT ? UPS_FILE_NOT_FOUND : UPS_IO_ERROR);
  }

  /* lock the file - this is default behaviour since 1.1.0 */
  lock_exclusive(fd, true);

  /* enable O_LARGEFILE support */
  enable_largefile(fd);

  m_fd = fd;
}

//
//
//
void File::close()
{
  if (m_fd != UPS_INVALID_FD) {
    // on posix, we most likely don't want to close descriptors 0 and 1
    assert(m_fd != 0 && m_fd != 1);

    // unlock the file - this is default behaviour since 1.1.0
    lock_exclusive(m_fd, false);

    // now close the descriptor
    if (::close(m_fd) == -1)
      throw Exception(UPS_IO_ERROR);

    m_fd = UPS_INVALID_FD;
  }
}


}
