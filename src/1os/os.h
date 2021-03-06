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
 * Abstraction layer for operating system functions
 */

#ifndef UPS_OS_H
#define UPS_OS_H

#include "0root/root.h"

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

namespace upscaledb {


#undef UPS_INVALID_FD
#define UPS_INVALID_FD  (-1)


// Returns true if the CPU supports AVX
extern bool
os_has_avx();

} // namespace upscaledb

#endif /* UPS_OS_H */
