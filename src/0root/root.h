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
 * The root of all evil. This header file must be included *before all others*!
 */

#ifndef UPS_ROOT_H
#define UPS_ROOT_H

//#define UPS_ENABLE_HELGRIND       1

// some feature macros in config.h must be set *before* inclusion
// of any system headers to have the desired effect.
// assume sane default values if there is no config.h.
#ifdef HAVE_CONFIG_H
#   include "../config.h"
#else
#   define HAVE_SCHED_YIELD       1
#   define HAVE_USLEEP            1
#endif

#include "ups/types.h"



// use tcmalloc?
#if HAVE_GOOGLE_TCMALLOC_H == 1
#  if HAVE_LIBTCMALLOC_MINIMAL == 1
#    define UPS_USE_TCMALLOC 1
#  endif
#endif

#include <stddef.h>
#define OFFSETOF(type, member) offsetof(type, member)

// helper macros to improve CPU branch prediction
#if defined __GNUC__
#   define likely(x) __builtin_expect ((x), 1)
#   define unlikely(x) __builtin_expect ((x), 0)
#else
#   define likely(x) (x)
#   define unlikely(x) (x)
#endif


// some compilers define min and max as macros; this leads to errors
// when using std::min and std::max
#ifdef min
#  undef min
#endif

#ifdef max
#  undef max
#endif

// helper macros for handling bitmaps with flags
#define ISSET(f, b)       (((f) & (b)) == (b))
#define ISSETANY(f, b)    (((f) & (b)) != 0)
#define NOTSET(f, b)      (((f) & (b)) == 0)

#endif // UPS_ROOT_H
