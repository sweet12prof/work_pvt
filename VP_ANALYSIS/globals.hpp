#pragma once
/*
 * We want to check distribution of
 * strided  Load patterns at a particular pc
 * irregular Load patterns at a particualr pc, that do not repeat.....difficult
 * to predict overall -> truly irregular patterns irregular Load patterns at a
 * particular pc that repeat -> irregular but repetitive patterns
 * intra-correlated irregular load aka Global Correlation -> At the individual
 * pc, it is irregular but it relates well with nearby loads; most likely RDS
 * loads Control Correlated loads ie; A memory access pattern within a function,
 * may be stride, truly irregular or irregular but repetitive The memory access
 * pattern within the function has a particular memory access pattern depending
 * on where the function was called Array based RDS structure; These are array
 * based recursive data structures For each address based predictor Check how
 * well they detect each pattern - Ideal , No storage limits Check how well they
 * detect each pattern - Realistic, with Limits Analyse Results Does one have a
 * good average for these types of patterns why ? Is there one that performs
 * well across all patterns Using a tournament approach Create inter
 * combinations Analyse Invesitigate a possible TAGE type that combines all.
 * Conflictng store disambiguation
 * Limitting access pattern type
 */
#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <access_pattern_define.hpp>
#include <array>
#include <map>
#include <types.h>
#include <vector>

#define CHUNK_SIZE 1000
#define STRIDE_RATIO_LOWER_THRESHOLD 5
#define METRIC_RATIO 5
enum class InstType : UINT8 { MEM_LOAD, MEM_STORE };

struct CallFrame {
  ADDRINT func_id;
  UINT8 has_meta;
  ADDRINT call_site;
};

struct TraceData {
  ADDRINT pc;
  InstType inst_type;
  CallFrame frame;
  ADDRINT memAddress;
};

struct Classification_Metrics {
  double dominant_stride_ratio, zero_stride_ratio, entropy, variance,
      number_of_strides, sign_symmetry_ratio, mean;
};

// struct Stride_Frequency_Distribution{
//     UINT64 frequency;
//     Classification_Metrics metrics;
// };

struct Stride_Dist_Struct {
  UINT64 dynamic_pc_count;
  UINT64 last_memaddress;
  UINT64 first_memaddress;
  INT64 current_stride;
  Classification_Metrics metrics;
  Acccess_Pattern_Type dominant_access_pattern_per_static_pc;
  std::map<INT64, UINT64> stride_freq;
};

struct Unqiue_Stride_Dist {
  std::vector<double> unique_strides;
  std::vector<double> frequencies;
};

struct AccessTypeCounts {
  UINT64 stridedAccessesFrequency;
  UINT64 trulyIrregularFrequency;
  UINT64 IrregularButRepetitiveFrequency;
};

struct Metrics {
  UINT64 global_load_count;
  UINT64 global_store_count;
  /* window of load accesses, before an intermediate conflicting store  */
  std::array<UINT64, 1000> loadIntervalFrequency;
  UINT64 ControlCorrelatedAccessCount;
  UINT64 GlobalCorrelatedAccessCount;
};

#endif
