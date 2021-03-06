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
 * btree find/insert/erase statistical structures, functions and macros
 */

#ifndef UPS_BTREE_STATS_H
#define UPS_BTREE_STATS_H

#include "0root/root.h"

#include <limits>

#include "ups/upscaledb_int.h"

// Always verify that a file of level N does not include headers > N!

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

namespace upscaledb {

class Page;

struct BtreeStatistics {
  // Indices into find/insert/erase specific statistics
  enum {
    kOperationFind      = 0,
    kOperationInsert    = 1,
    kOperationErase     = 2,
    kOperationMax       = 3
  };

  struct FindHints {
    // the original flags of ups_find
    uint32_t original_flags;

    // the modified flags
    uint32_t flags;

    // page/btree leaf to check first
    uint64_t leaf_page_addr;

    // check specified btree leaf node page first
    bool try_fast_track;
  };

  struct InsertHints {

    InsertHints()
        : original_flags( 0 )
        , flags( 0)
        , leaf_page_addr( 0 )
        , processed_leaf_page( nullptr )
        , processed_slot( 0 )
        , append_count( 0 )
        , prepend_count( 0 )
    {

    }

    InsertHints(uint32_t _original_flags, uint32_t _flags, uint64_t _leaf_page_addr, Page *_processed_leaf_page,
                uint16_t _processed_slot, size_t _append_count, size_t _prepend_count)
        : original_flags( _original_flags )
        , flags( _flags )
        , leaf_page_addr( _leaf_page_addr )
        , processed_leaf_page( _processed_leaf_page )
        , processed_slot( _processed_slot )
        , append_count( _append_count )
        , prepend_count( _prepend_count )
    {

    }

    // the original flags of ups_insert
    uint32_t original_flags;

    // the modified flags
    uint32_t flags;

    // page/btree leaf to check first
    uint64_t leaf_page_addr;

    // the processed leaf page
    Page *processed_leaf_page;

    // the slot in that page
    uint16_t processed_slot;

    // count the number of appends
    size_t append_count;

    // count the number of prepends
    size_t prepend_count;
  };

  // Constructor
  BtreeStatistics();

  // Returns the btree hints for ups_find
  FindHints find_hints(uint32_t flags);

  // Returns the btree hints for insert
  InsertHints insert_hints(uint32_t flags);

  // Reports that a ups_find/ups_cusor_find succeeded
  void find_succeeded(Page *page);

  // Reports that a ups_find/ups_cursor_find failed
  void find_failed();

  // Reports that a ups_insert/ups_cursor_insert succeeded
  void insert_succeeded(Page *page, uint16_t slot);

  // Reports that a ups_insert/ups_cursor_insert failed
  void insert_failed();

  // Reports that a ups_erase/ups_cusor_erase succeeded
  void erase_succeeded(Page *page);

  // Reports that a ups_erase/ups_cursor_erase failed
  void erase_failed();

  // Keep track of the KeyList range size
  void set_keylist_range_size(bool leaf, size_t size) {
    state.keylist_range_size[(int)leaf] = size;
  }

  // Retrieves the KeyList range size
  size_t keylist_range_size(bool leaf) const {
    return state.keylist_range_size[(int)leaf];
  }

  // Keep track of the KeyList capacities
  void set_keylist_capacities(bool leaf, size_t capacity) {
    state.keylist_capacities[(int)leaf] = capacity;
  }

  // Retrieves the KeyList capacities size
  size_t keylist_capacities(bool leaf) const {
    return state.keylist_capacities[(int)leaf];
  }

  // Calculate the "average" values
  static void finalize_metrics(btree_metrics_t *metrics);

  // Update a min_max_avg structure
  static void update_min_max_avg(min_max_avg_u32_t *data, uint32_t value) {
    // first update? then perform initialization
    if (data->_instances == 0)
      data->min = std::numeric_limits<uint32_t>::max();

    if (data->min > value)
      data->min = value;
    if (data->max < value)
      data->max = value;
    data->_total += value;
    data->_instances++;
  }

  struct BtreeStatsState {
    // last leaf page for find/insert/erase
    uint64_t last_leaf_pages[kOperationMax];

    // count of how often this leaf page was used
    size_t last_leaf_count[kOperationMax];

    // count the number of appends
    size_t append_count;

    // count the number of prepends
    size_t prepend_count;

    // the range size of the KeyList
    size_t keylist_range_size[2];

    // the capacities of the KeyList
    size_t keylist_capacities[2];
  } state;
};

} // namespace upscaledb

#endif // UPS_BTREE_STATS_H
