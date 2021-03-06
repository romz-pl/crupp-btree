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

#include "0root/root.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Always verify that a file of level N does not include headers > N!
#include "1globals/globals.h"

#ifndef UPS_ROOT_H
#  error "root.h was not included"
#endif

namespace upscaledb {

static int
dbg_snprintf(char *str, size_t size, const char *format, ...)
{
  int s;

  va_list ap;
  va_start(ap, format);
  s = vsnprintf(str, size, format, ap);
  va_end(ap);

  return s;
}

void 
default_errhandler(int level, const char *message)
{
#ifdef NDEBUG
  if (level == UPS_DEBUG_LEVEL_DEBUG)
    return;
#endif
  fprintf(stderr, "%s\n", message);
}

void
dbg_prepare(int level, const char *file, int line, const char *function,
            const char *expr)
{
  Globals::ms_error_level = level;
  Globals::ms_error_file = file;
  Globals::ms_error_line = line;
  Globals::ms_error_expr = expr;
  Globals::ms_error_function = function;
}

void
dbg_log(const char *format, ...)
{
  int s = 0;
  char buffer[1024 * 4];

  va_list ap;
  va_start(ap, format);
#ifdef UPS_DEBUG
  s = dbg_snprintf(buffer,   sizeof(buffer), "%s[%d]: ",
                  Globals::ms_error_file, Globals::ms_error_line);
  util_vsnprintf(buffer + s, sizeof(buffer) - s, format, ap);
#else
  if (Globals::ms_error_function)
    s = dbg_snprintf(buffer, sizeof(buffer), "%s: ",
                    Globals::ms_error_function);
  vsnprintf(buffer + s, sizeof(buffer) - s, format, ap);
#endif
  va_end(ap);

  Globals::ms_error_handler(Globals::ms_error_level, buffer);
}

} // namespace upscaledb

