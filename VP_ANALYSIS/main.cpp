#include <analyser.hpp>
#include <fstream>
#include <gzstream.h>
#include <string>
#include <utils.hpp>
#include <vector>
#ifndef TRACE_LOC
#define TRACE_LOC ""
#endif
#ifndef LOG_LOC
#define LOG_LOC
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
    Analyser analyser(&file);
    analyser.process_trace();
    analyser.gen_classifiction_metrics();
    analyser.print_per_pc_stride_dist(&logfile);
  }
}
