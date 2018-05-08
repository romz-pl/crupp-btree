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

#ifndef UPS_PAGE_H
#define UPS_PAGE_H

#include <cstdint>

#include "1base/spinlock.h"
#include "1base/intrusive_list.h"
#include "persisted_data.h"

namespace upscaledb {

struct Device;
struct BtreeCursor;
struct BtreeNodeProxy;
struct LocalDb;


class Page
{
public:

    // Misc. enums
    enum
    {
        // sizeof the persistent page header
        kSizeofPersistentHeader = sizeof( PPageHeader ) - 1,

        // instruct Page::alloc() to reset the page with zeroes
        kInitializeWithZeroes,
    };

    // The various linked lists (indices in m_prev, m_next)
    enum
    {
        // list of all cached pages
        kListCache              = 0,

        // list of all pages in a changeset
        kListChangeset          = 1,

        // a bucket in the hash table of the cache
        kListBucket             = 2,

        // array limit
        kListMax                = 3
    };

    // non-persistent page flags
    enum
    {
        // page->m_data was allocated with malloc, not mmap
        kNpersMalloc            = 1,

        // page has no header (i.e. it's part of a large blob)
        kNpersNoHeader          = 2
    };

    // Page types
    //
    // When large BLOBs span multiple pages, only their initial page
    // will have a valid type code; subsequent pages of this blog will store
    // the data as-is, so as to provide one continuous storage space
    enum
    {
        // unidentified db page type
        kTypeUnknown            =  0x00000000,

        // the header page: this is the first page in the environment (offset 0)
        kTypeHeader             =  0x10000000,

        // a B+tree root page
        kTypeBroot              =  0x20000000,

        // a B+tree node page
        kTypeBindex             =  0x30000000,

        // a page storing the state of the PageManager
        kTypePageManager        =  0x40000000,

        // a page which stores blobs
        kTypeBlob               =  0x50000000
    };


    Page( Device *device, LocalDb *db = nullptr );
    ~Page();


    uint32_t usable_page_size();
    Spinlock &mutex();


    LocalDb *db();
    void set_db( LocalDb *db );

    uint64_t address() const;
    void set_address( uint64_t address );

    uint32_t type() const;
    void set_type( uint32_t type );

    uint32_t crc32() const;
    void set_crc32( uint32_t crc32 );

    uint64_t lsn() const;
    void set_lsn( uint64_t lsn );

    PPageData* data();
    void set_data( PPageData *data );

    uint8_t* payload();
    uint8_t* raw_payload();

    bool is_header() const;

    bool is_dirty() const;
    void set_dirty( bool dirty );

    bool is_allocated() const;
    bool is_without_header() const;
    void set_without_header( bool is_without_header );

    void assign_allocated_buffer( void *buffer, uint64_t address );
    void assign_mapped_buffer( void *buffer, uint64_t address );

    void free_buffer();
    void alloc( uint32_t type, uint32_t flags = 0 );
    void fetch( uint64_t address );
    void flush();

    BtreeNodeProxy *node_proxy();
    void set_node_proxy( BtreeNodeProxy* proxy );

    Page *next( int list );
    Page *previous( int list );

public:

    // tracks number of flushed pages
    static uint64_t ms_page_count_flushed;

    // the persistent data of this page
    PersistedData persisted_data;

    // Intrusive linked lists
    IntrusiveListNode< Page, Page::kListMax > list_node;

    // Intrusive linked btree cursors
    IntrusiveList< BtreeCursor > cursor_list;

private:
    // the Device for allocating storage
    Device* device_;

    // the Database handle (can be NULL)
    LocalDb* db_;

    // the cached BtreeNodeProxy object
    BtreeNodeProxy* node_proxy_;
};

} // namespace upscaledb

#endif /* UPS_PAGE_H */
