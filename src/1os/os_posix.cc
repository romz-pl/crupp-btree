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

#define _GNU_SOURCE        1 // for O_LARGEFILE
#define _FILE_OFFSET_BITS 64

#include "0root/root.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#if HAVE_MMAP
#  include <sys/mman.h>
#endif
#if HAVE_WRITEV
#  include <sys/uio.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

// Always verify that a file of level N does not include headers > N!
#include "1base/error.h"
#include "1errorinducer/errorinducer.h"
#include "1os/file.h"
#include "1os/socket.h"

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

