#include <fstream>
#include <string>

#ifndef TRACE_LOC
#define TRACE_LOC ""
#endif
#ifndef LOG_LOC
#define LOG_LOC
#endif

#ifndef RUN_LOC
#define RUN_LOC ""
#endif

#ifndef LOG_FILE_DEFINE_H
#define LOG_FILE_DEFINE_H

#define LOG_FILE_DEFINE(var_name, var_value) \
  std::ofstream var_name##_FILE(RUN_LOC #var_value, std::ios::out);
#include "log_filenames_table.def"
#undef LOG_FILE_DEFINE

#endif