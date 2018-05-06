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


/**
 * @file types.h
 * @brief Portable typedefs for upscaledb
 * @author Christoph Rupp, chris@crupp.de
 *
 */

#ifndef UPS_TYPES_H
#define UPS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Check the operating system and word size
 */
#ifdef WIN32
#  undef  UPS_OS_WIN32
#  define UPS_OS_WIN32 1
#  ifdef WIN64
#    undef  UPS_64BIT
#    define UPS_64BIT 1
#  elif WIN32
#    undef  UPS_32BIT
#    define UPS_32BIT 1
#  else
#    error "Neither WIN32 nor WIN64 defined!"
#  endif
#else /* posix? */
#  undef  UPS_OS_POSIX
#  define UPS_OS_POSIX 1
#  if defined(__LP64__) || defined(__LP64) || __WORDSIZE == 64
#    undef  UPS_64BIT
#    define UPS_64BIT 1
#  else
#    undef  UPS_32BIT
#    define UPS_32BIT 1
#  endif
#endif

#if defined(UPS_OS_POSIX) && defined(UPS_OS_WIN32)
#  error "Unknown arch - neither UPS_OS_POSIX nor UPS_OS_WIN32 defined"
#endif


/*
 * Common typedefs. Since stdint.h is not available on older versions of
 * Microsoft Visual Studio, they get declared here.
 * http://msinttypes.googlecode.com/svn/trunk/stdint.h
 */
#if _MSC_VER
#  include <ups/msstdint.h>
#else
#  include <stdint.h>
#endif

/*
 * Undefine macros to avoid macro redefinitions
 */
#undef UPS_INVALID_FD
#undef UPS_FALSE
#undef UPS_TRUE

/**
 * a boolean type
 */
typedef int                ups_bool_t;
#define UPS_FALSE          0
#define UPS_TRUE           (!UPS_FALSE)

/**
 * typedef for error- and status-code
 */
typedef int                ups_status_t;


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* UPS_TYPES_H */
