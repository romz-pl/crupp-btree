#ifndef UPS_PERSISTED_DATA_H
#define UPS_PERSISTED_DATA_H

#include <cstdint>
#include "1base/spinlock.h"
#include "ppage_data.h"

namespace upscaledb {

// A wrapper around the persisted page data
struct PersistedData
{
    PersistedData()
        : address(0), size(0), is_dirty(false), is_allocated(false),
          is_without_header(false), raw_data(0) {
    }

    PersistedData(const PersistedData &other)
        : address(other.address), size(other.size), is_dirty(other.is_dirty),
          is_allocated(other.is_allocated),
          is_without_header(other.is_without_header), raw_data(other.raw_data) {
    }

    ~PersistedData() {
#ifdef NDEBUG
        mutex.safe_unlock();
#endif
        if (is_allocated)
            Memory::release(raw_data);
        raw_data = 0;
    }

    // The spinlock is locked if the page is in use or written to disk
    Spinlock mutex;

    // address of this page - the absolute offset in the file
    uint64_t address;

    // the size of this page
    uint32_t size;

    // is this page dirty and needs to be flushed to disk?
    bool is_dirty;

    // Page buffer was allocated with malloc() (if not then it was mapped
    // with mmap)
    bool is_allocated;

    // True if page has no persistent header
    bool is_without_header;

    // the persistent data of this page
    PPageData *raw_data;
};



}



#endif
