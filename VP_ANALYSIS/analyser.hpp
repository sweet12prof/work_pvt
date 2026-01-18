#include <globals.hpp>
#include <iostream>
#include <vector>
namespace Address_Analysis {
class Analyser {
public:
  Analyser(std::istream *file);
  void gen_pc_stide_distribution();
  void gen_classifiction_metrics();
  void process_trace();
  void print_per_pc_stride_dist(std::ostream *out);

private:
  std::pair<bool, std::vector<TraceData>> traceChunk;
  std::istream *file;
  std::map<UINT64, Stride_Dist_Struct> per_pc_dist;
  void get_per_pc_dominant_stride_ratio();

  UINT64 loads_processed;
  UINT64 accesses_count{0};
  // Metrics metrics;
};

double get_dominant_stride_ratio(const std::vector<UINT64> &freq_vec,
                                 UINT64 dynamic_load_count);

inline double get_zero_stride_ratio(UINT64 zero_stride_freq,
                                    UINT64 dynamic_load_count) {
  return dynamic_load_count < STRIDE_RATIO_LOWER_THRESHOLD
             ? 0
             : static_cast<double>(zero_stride_freq) / dynamic_load_count;
}
/*
  Some patterns go from -(ve) phases to +(ve) phases, others just a single this
  means
    * +(ve) only mean -> strides were mostly positive
    * -(ve) only means pattern  -> strides were mostly negative
    * close to zero -> positive and negative strides roughly cancel out. Tree
  traversal ?
*/
double get_mean(const std::vector<std::pair<INT64, UINT64>> &freq_vec,
                UINT64 dynamic_load_count);

double get_variance(const std::vector<std::pair<INT64, UINT64>> &freq_vec,
                    double mean, UINT64 dynamic_load_count);

double get_normalized_entropy(const std::vector<UINT64> &freq_vec,
                              UINT64 dynamic_load_count);

std::vector<UINT64> get_per_stride_freq_vec(const std::map<INT64, UINT64> &);
}; // namespace Address_Analysis
