#include <limits>
#include "env_config.h"

namespace upscaledb {

// the default cache size is 2 MB
const uint64_t EnvConfig::UPS_DEFAULT_CACHE_SIZE = ( 2 * 1024 * 1024 );

// the default page size is 16 kb
const uint32_t EnvConfig::UPS_DEFAULT_PAGE_SIZE = ( 16 * 1024 );

// Value for @ref UPS_PARAM_POSIX_FADVISE
const int EnvConfig::UPS_POSIX_FADVICE_NORMAL = 0;

// Value for @ref UPS_PARAM_POSIX_FADVISE
const int EnvConfig::UPS_POSIX_FADVICE_RANDOM = 1;

EnvConfig::EnvConfig()
    : flags( 0 )
    , file_mode( 0644 )
    , max_databases( 0 )
    , page_size_bytes( UPS_DEFAULT_PAGE_SIZE )
    , cache_size_bytes( UPS_DEFAULT_CACHE_SIZE )
    , file_size_limit_bytes( std::numeric_limits< size_t >::max() )
    , remote_timeout_sec( 0 )
    , journal_compressor( 0 )
    , is_encryption_enabled( false )
    , journal_switch_threshold( 0 )
    , posix_advice( UPS_POSIX_FADVICE_NORMAL )
{
}

}

