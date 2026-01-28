#ifndef ACCESS_PATTERN_DEFINE_HPP
#define ACCESS_PATTERN_DEFINE_HPP

enum class Acccess_Pattern_Type {
#define ACCESS_PATTERN_DEF(id, name) id,
#include "access_pattern_table.def"
#undef ACCESS_PATTERN_DEF
  UNCLASSIFIED_PATTERN
};

extern const char *Acccess_Pattern_Names[];

#endif