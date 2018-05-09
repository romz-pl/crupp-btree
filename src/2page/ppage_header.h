#ifndef UPS_PPAGE_HEADER_H
#define UPS_PPAGE_HEADER_H

#include <stdint.h>


namespace upscaledb {

#include "1base/packstart.h"

/*
 * This header is only available if the (non-persistent) flag
 * kNpersNoHeader is not set! Blob pages do not have this header.
 */
typedef UPS_PACK_0 struct UPS_PACK_1 PPageHeader
{
    // flags of this page - currently only used for the Page::kType* codes
    uint32_t flags;

    // crc32
    uint32_t crc32;

    // the lsn of the last operation
    uint64_t lsn;

    // the persistent data blob
    uint8_t payload[ 1 ];

} UPS_PACK_2 PPageHeader;

#include "1base/packstop.h"

}

#endif

