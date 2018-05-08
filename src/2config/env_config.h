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
 * The configuration settings of an Environment.
 *
 * @exception_safe nothrow
 * @thread_safe no
 */

#ifndef UPS_ENV_CONFIG_H
#define UPS_ENV_CONFIG_H

#include <cstddef>
#include <string>

namespace upscaledb {

struct EnvConfig
{
    EnvConfig();

    // the environment's flags
    uint32_t flags;

    // the file mode
    int file_mode;

    // the number of databases
    int max_databases;

    // the page size (in bytes)
    uint32_t page_size_bytes;

    // the cache size (in bytes)
    uint64_t cache_size_bytes;

    // the file size limit (in bytes)
    size_t file_size_limit_bytes;

    // the remote timeout (in seconds)
    size_t remote_timeout_sec;

    // the path (or remote location)
    std::string filename;

    // the path of the logfile
    std::string log_filename;

    // the algorithm for journal compression
    int journal_compressor;

    // true if AES encryption is enabled
    bool is_encryption_enabled;

    // the AES encryption key
    uint8_t encryption_key[ 16 ];

    // threshold for switching journal files
    size_t journal_switch_threshold;

    // parameter for posix_fadvise()
    uint32_t posix_advice;

public:
    // the default cache size is 2 MB
    static const uint64_t UPS_DEFAULT_CACHE_SIZE;

    // the default page size is 16 kb
    static const uint32_t UPS_DEFAULT_PAGE_SIZE;

    // Value for @ref UPS_PARAM_POSIX_FADVISE
    static const uint32_t UPS_POSIX_FADVICE_NORMAL;

    // Value for @ref UPS_PARAM_POSIX_FADVISE
    static const uint32_t UPS_POSIX_FADVICE_RANDOM;
};

} // namespace upscaledb

#endif // UPS_ENV_CONFIG_H
