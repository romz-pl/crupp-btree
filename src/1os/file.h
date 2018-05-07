/*
 * Copyright (C) 2005-2017 Christoph Rupp (chris@crupp.de).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * See the file COPYING for License information.
 */

/*
 * A simple wrapper around a file handle. Throws exceptions in
 * case of errors. Moves the file handle when copied.
 */

#ifndef UPS_FILE_H
#define UPS_FILE_H

#include "0root/root.h"

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

namespace upscaledb {

class File
{
public:
    File();
    File( File&& other );
    File& operator=( File &&other );
    ~File();

    void create( const char *filename, uint32_t mode );
    void open( const char *filename, bool read_only );
    void close();

    bool is_open() const;

    void flush();

    void set_posix_advice( int parameter );

    void mmap( uint64_t position, size_t size, bool readonly, uint8_t **buffer );
    void munmap( void *buffer, size_t size );

    void pread( uint64_t addr, void *buffer, size_t len );
    void pwrite( uint64_t addr, const void *buffer, size_t len );
    void write( const void *buffer, size_t len );


    void seek( uint64_t offset, int whence ) const;
    uint64_t tell() const;
    uint64_t file_size() const;
    void truncate( uint64_t newsize );


    static size_t granularity();
    static void os_read( int fd, uint8_t *buffer, size_t len );
    static void os_write( int fd, const void *buffer, size_t len );

private:
    static void enable_largefile( int fd );
    static void lock_exclusive( int fd, bool lock );

  private:
    // The file handle
    int m_fd;

    // Parameter for posix_fadvise()
    int m_posix_advice;
};

} // namespace upscaledb

#endif /* UPS_FILE_H */
