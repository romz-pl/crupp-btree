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
 * Device-implementation for disk-based files. Exception safety is "strong"
 * for most operations, but currently it's possible that the Page is modified
 * if DiskDevice::read_page fails in the middle.
 *
 * @exception_safe: basic/strong
 * @thread_safe: no
 */

#ifndef UPS_DEVICE_DISK_H
#define UPS_DEVICE_DISK_H

#include <utility>

#include "0root/root.h"

#include "1base/spinlock.h"
#include "1os/file.h"
#include "2device/device.h"

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

namespace upscaledb {

/*
 * a File-based device
 */
class DiskDevice : public Device
{
    struct State
    {
        State() = default;
        State( const State& ) = delete;
        State& operator=( const State& ) = delete;

        State( State&& ) = default;
        State& operator=( State&& ) = default;

        // the database file
        File file;

        // pointer to the the mmapped data
        uint8_t *mmapptr;

        // the size of mmapptr as used in mmap
        uint64_t mapped_size;

        // the (cached) size of the file
        uint64_t file_size;

        // excess storage at the end of the file
        uint64_t excess_at_end;

        // Allow state to be swapped
        friend void swap( State& old_state, State& new_state )
        {
            std::swap( old_state, new_state );
        }
    };

public:
    DiskDevice( const EnvConfig &config );

    void create() override;
    void open() override;
    bool is_open() override;
    void close() override;
    void flush() override;
    void truncate( uint64_t new_file_size ) override;
    uint64_t file_size() override;
    void seek( uint64_t offset, int whence ) override;
    uint64_t tell() override;

    void read( uint64_t offset, void *buffer, size_t len ) override;
    void write( uint64_t offset, void *buffer, size_t len ) override;

    uint64_t alloc( size_t requested_length ) override;
    void read_page( Page *page, uint64_t address ) override;
    void alloc_page( Page *page ) override;
    void free_page( Page *page ) override;

    bool is_mapped( uint64_t file_offset, size_t size ) const override;
    void reclaim_space() override;

    uint8_t *mapped_pointer( uint64_t address ) const;

private:
    // truncate/resize the device, sans locking
    void truncate_nolock( uint64_t new_file_size );

private:
    // For synchronizing access
    Spinlock m_mutex;

    State m_state;
};

} // namespace upscaledb

#endif /* UPS_DEVICE_DISK_H */
