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
 * @exception_safe: strong
 * @thread_safe: no
 */

#ifndef UPS_DEVICE_INMEM_H
#define UPS_DEVICE_INMEM_H

#include "0root/root.h"
#include "2device/device.h"

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

namespace upscaledb {

/*
 * an In-Memory device
 */
class InMemoryDevice : public Device
{
public:

    InMemoryDevice( const EnvConfig &config );

    void create() override;
    void open() override;
    bool is_open() const override;
    void close() override;

    void flush() const override;
    void truncate( uint64_t ) override;

    uint64_t file_size() const override;
    void seek( uint64_t, int ) const override;
    uint64_t tell() const override;

    void read( uint64_t, void*, size_t ) const override;
    void write( uint64_t, void*, size_t ) const override;
    void read_page( Page*, uint64_t ) const override;

    uint64_t alloc( size_t size ) override;
    void alloc_page( Page *page ) override;
    void free_page( Page *page ) override;

    bool is_mapped( uint64_t, size_t ) const override;
    void reclaim_space() override;

    void release( void *ptr, size_t size );

private:
    // flag whether this device was "opened" or is uninitialized
    bool is_open_;

    // the allocated bytes
    uint64_t allocated_size_;
};

} // namespace upscaledb

#endif /* UPS_DEVICE_INMEM_H */
