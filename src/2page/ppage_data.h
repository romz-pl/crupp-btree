#ifndef UPS_PPAGE_DATA_H
#define UPS_PPAGE_DATA_H

#include <cstdint>
#include "ppage_header.h"

namespace upscaledb {

#include "1base/packstart.h"

/*
 * A union combining the page header and a pointer to the raw page data.
 *
 * This structure definition is present outside of @ref Page scope
 * to allow compile-time OFFSETOF macros to correctly judge the size,
 * depending on platform and compiler settings.
 */
typedef UPS_PACK_0 union UPS_PACK_1 PPageData
{
    // the persistent header
    PPageHeader header;

    // a char pointer to the allocated storage on disk
    uint8_t payload[ 1 ];

} UPS_PACK_2 PPageData;

#include "1base/packstop.h"

}

#endif
