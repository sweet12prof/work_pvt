// #include <access_pattern_define.hpp>
#include <algorithm>
#include <analyser.hpp>
#include <fstream>
#include <gzstream.h>
#include <matplot/matplot.h>
#include <string>
#include <utils.hpp>
#include <vector>

#ifndef TRACE_LOC
#define TRACE_LOC ""
#endif
#ifndef LOG_LOC
#define LOG_LOC
#endif

#ifndef RUN_LOC
#define RUN_LOC
#endif

int main() {
  std::string filename =
      std::string(TRACE_LOC) + "bwaves_0.ref_2092028.pp.trace";
  igzstream file{filename.c_str(), std::ios::binary | std::ios::in};
  std::string logfilename{std::string(LOG_LOC) + "log.txt"};
  std::ofstream logfile(logfilename, std::ios::out);
  if (!file) {
    std::cerr << "file not found, exiting ";
    return 1;
  }
  {
    using namespace Address_Analysis;
    using namespace matplot;
    Analyser analyser(&file);
    analyser.process_trace();
    analyser.gen_classifiction_metrics();
    analyser.print_per_pc_stride_dist(&logfile);

    auto general_stride_dist{analyser.get_all_unique_strides_distribution()};

    std::vector<double> log_2_strides(
        general_stride_dist.unique_strides.size());
    std::vector<double> log_10_frequencies(
        general_stride_dist.unique_strides.size());

    std::transform(general_stride_dist.unique_strides.begin(),
                   general_stride_dist.unique_strides.end(),
                   log_2_strides.begin(),
                   [](double val) { return std::log2(val); });

    std::transform(general_stride_dist.frequencies.begin(),
                   general_stride_dist.frequencies.end(),
                   log_10_frequencies.begin(),
                   [](double val) { return std::log10(val); });

    scatter(log_2_strides, log_10_frequencies);
    xlabel("log2(stride)");
    ylabel("log(frequency)");
    std::string file_loc = std::string(RUN_LOC) + "unqiue_stride_dist";

    save(file_loc, "png");
  }
}
