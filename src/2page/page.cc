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

#include "0root/root.h"

#include <cstring>
#include "3rdparty/murmurhash3/MurmurHash3.h"

#include "1base/error.h"
#include "1os/os.h"
#include "2page/page.h"
#include "2device/device.h"
#include "3btree/btree_node_proxy.h"

namespace upscaledb {

uint64_t Page::ms_page_count_flushed = 0;

//
// Default constructor
//
Page::Page( Device *device, LocalDb *db )
    : device_( device )
    , db_( db )
    , node_proxy_( nullptr )
{
    persisted_data.raw_data = 0;
    persisted_data.is_dirty = false;
    persisted_data.is_allocated = false;
    persisted_data.address = 0;
    persisted_data.size = (uint32_t)device->page_size();
}

//
// Destructor - releases allocated memory and resources, but neither
// flushes dirty pages to disk nor moves them to the freelist!
// Asserts that no cursors are attached.
//
Page::~Page()
{
    assert( cursor_list.is_empty() );
    free_buffer();
}

//
// Returns the size of the usable persistent payload of a page
// (page_size minus the overhead of the page header)
//
uint32_t Page::usable_page_size()
{
    uint32_t raw_page_size = db_->env->config.page_size_bytes;
    return raw_page_size - Page::kSizeofPersistentHeader;
}

//
//
//
void Page::alloc(uint32_t type, uint32_t flags)
{
    device_->alloc_page( this );

    if( flags & kInitializeWithZeroes )
    {
        size_t page_size = device_->page_size();
        ::memset( raw_payload(), 0, page_size );
    }

    if( type )
    {
        set_type( type );
    }
}

//
//
//
void Page::fetch( uint64_t address )
{
    device_->read_page( this, address );
    set_address( address );
}

//
//
//
void Page::flush()
{
    if( persisted_data.is_dirty )
    {
        // update crc32
        if( IS_SET( device_->config.flags, UPS_ENABLE_CRC32 ) && likely( !persisted_data.is_without_header ) )
        {
            MurmurHash3_x86_32( persisted_data.raw_data->header.payload,
                                persisted_data.size - (sizeof(PPageHeader) - 1),
                                (uint32_t)persisted_data.address,
                                &persisted_data.raw_data->header.crc32 );
        }
        device_->write( persisted_data.address, persisted_data.raw_data, persisted_data.size );
        persisted_data.is_dirty = false;
        ms_page_count_flushed++;
    }
}

//
//
//
void Page::free_buffer()
{
    if( node_proxy_ )
    {
        delete node_proxy_;
        node_proxy_ = 0;
    }
}

//
// Returns the spinlock
//
Spinlock& Page::mutex()
{
    return persisted_data.mutex;
}

//
// Returns the database which manages this page; can be NULL if this
// page belongs to the Environment (i.e. for freelist-pages)
//
LocalDb* Page::db()
{
    return db_;
}

//
// Sets the database to which this Page belongs
//
void Page::set_db( LocalDb *db )
{
    db_ = db;
}

//
// Returns the address of this page
//
uint64_t Page::address() const
{
    return persisted_data.address;
}

//
// Sets the address of this page
//
void Page::set_address( uint64_t address )
{
    persisted_data.address = address;
}

//
// Returns the page's type (kType*)
//
uint32_t Page::type() const
{
    return persisted_data.raw_data->header.flags;
}

//
// Sets the page's type (kType*)
//
void Page::set_type( uint32_t type )
{
    persisted_data.raw_data->header.flags = type;
}

//
// Returns the crc32
//
uint32_t Page::crc32() const
{
    return persisted_data.raw_data->header.crc32;
}

//
// Sets the crc32
//
void Page::set_crc32( uint32_t crc32 )
{
    persisted_data.raw_data->header.crc32 = crc32;
}

//
// Returns the lsn
//
uint64_t Page::lsn() const
{
    return persisted_data.raw_data->header.lsn;
}

//
// Sets the lsn
//
void Page::set_lsn( uint64_t lsn )
{
    persisted_data.raw_data->header.lsn = lsn;
}

//
// Returns the pointer to the persistent data
//
PPageData* Page::data()
{
    return persisted_data.raw_data;
}

//
// Sets the pointer to the persistent data
//
void Page::set_data( PPageData* data )
{
    persisted_data.raw_data = data;
}

//
// Returns the persistent payload (after the header!)
//
uint8_t* Page::payload()
{
    return persisted_data.raw_data->header.payload;
}

//
// Returns the persistent payload (including the header!)
//
uint8_t* Page::raw_payload()
{
    return persisted_data.raw_data->payload;
}

//
// Returns true if this is the header page of the Environment
//
bool Page::is_header() const
{
    return persisted_data.address == 0;
}

//
// Returns true if this page is dirty (and needs to be flushed to disk)
//
bool Page::is_dirty() const
{
    return persisted_data.is_dirty;
}

//
// Sets this page dirty/not dirty
//
void Page::set_dirty( bool dirty )
{
    persisted_data.is_dirty = dirty;
}

//
// Returns true if the page's buffer was allocated with malloc
//
bool Page::is_allocated() const
{
    return persisted_data.is_allocated;
}

//
// Returns true if the page has no persistent header
//
bool Page::is_without_header() const
{
    return persisted_data.is_without_header;
}

//
// Sets the flag whether this page has a persistent header or not
//
void Page::set_without_header( bool is_without_header )
{
    persisted_data.is_without_header = is_without_header;
}

//
// Assign a buffer which was allocated with malloc()
//
void Page::assign_allocated_buffer( void* buffer, uint64_t address )
{
    free_buffer();
    persisted_data.raw_data = (PPageData *)buffer;
    persisted_data.is_allocated = true;
    persisted_data.address = address;
}

//
// Assign a buffer from mmapped storage
//
void Page::assign_mapped_buffer( void* buffer, uint64_t address )
{
    free_buffer();
    persisted_data.raw_data = (PPageData *)buffer;
    persisted_data.is_allocated = false;
    persisted_data.address = address;
}

//
// Returns the cached BtreeNodeProxy
//
BtreeNodeProxy* Page::node_proxy()
{
    return node_proxy_;
}

//
// Sets the cached BtreeNodeProxy
//
void Page::set_node_proxy( BtreeNodeProxy* proxy )
{
    node_proxy_ = proxy;
}

//
// Returns the next page in a linked list
//
Page* Page::next( int list )
{
    return list_node.next[ list ];
}

//
// Returns the previous page of a linked list
//
Page* Page::previous( int list )
{
    return list_node.previous[ list ];
}



} // namespace upscaledb
