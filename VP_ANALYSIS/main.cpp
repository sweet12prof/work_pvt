// #include <access_pattern_define.hpp>
#include <gzstream.h>

#include <algorithm>
#include <analyser.hpp>
#include <fstream>

#include "GlobalAnalyser.hpp"
#include "fileprocess.hpp"
// #include <matplot/matplot.h>
#include <string>
#include <utils.hpp>
#include <vector>

#include "log_file_defines.hpp"

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
      std::string(TRACE_LOC) + "traces/bwaves_0.ref_2092028.pp.trace";

  std::string filename1 =
      std::string(TRACE_LOC) + "traces/bwaves_0.ref_2092028.pp1.trace";

  std::cout << std::endl << "Location is " << filename << std::endl;
  igzstream file2{filename.c_str(), std::ios::binary | std::ios::in};
  igzstream file3{filename1.c_str(), std::ios::binary | std::ios::in};

  if (!file2) {
    std::cout << "file not found, exiting " << std::endl;
    return 1;
  }
  {
    using namespace Static_Address_Analysis;
    // using namespace matplot;
    // Analyser analyser(&file);
    // analyser.process_trace();
    // analyser.gen_classifiction_metrics();
    // analyser.print_per_pc_stride_dist(&LOG_FILE);

    // auto general_stride_dist{analyser.get_all_unique_strides_distribution()};

    // std::vector<double> log_2_strides(
    //     general_stride_dist.unique_strides.size());

    // std::vector<double> log_10_frequencies(
    //     general_stride_dist.unique_strides.size());

    // std::transform(general_stride_dist.unique_strides.begin(),
    //                general_stride_dist.unique_strides.end(),
    //                log_2_strides.begin(),
    //                [](double val) { return std::log2(val); });

    // std::transform(general_stride_dist.frequencies.begin(),
    //                general_stride_dist.frequencies.end(),
    //                log_10_frequencies.begin(),
    //                [](double val) { return std::log10(val); });

    // scatter(log_2_strides, log_10_frequencies);
    // xlabel("log2(stride)");
    // ylabel("log(frequency)");
    // std::string file_loc = std::string(RUN_LOC) + "unqiue_stride_dist";

    FileProcess f(&file2), f2(&file3);

    GlobalAnalyser_FT pc_graph, mem_graph;
    std::pair<bool, std::vector<TraceData>> res;
    std::vector<TraceData> leftOver;
    std::vector<TraceData> chunkforhash;
    std::vector<TraceData> chunkforgraph;

    TraceData last_element;
    bool has_last_element{false};
    //= f.progress_read();
    do {
      res = f.progress_read();
      if (has_last_element && !res.second.empty()) {
        mem_graph.add_single_transition(last_element, res.second.front(),
                                        Vertex_Property::MEM_ADDRESS);
        pc_graph.add_single_transition(last_element, res.second.front(),
                                       Vertex_Property::PC);
      }
      mem_graph.fill_graph(res.second, Vertex_Property::MEM_ADDRESS);
      pc_graph.fill_graph(res.second, Vertex_Property::PC);
    } while (!res.first);
    pc_graph.generate_strongly_connected_componenets();
    mem_graph.generate_strongly_connected_componenets();

    do {
      res = f2.progress_read();
      if (leftOver.empty())
        pc_graph.hash_windows(res.second, 16, Vertex_Property::PC, leftOver);
      else {
        chunkforhash = leftOver;
        chunkforhash.insert(chunkforhash.end(), res.second.begin(),
                            res.second.end());
        pc_graph.hash_windows(chunkforhash, 16, Vertex_Property::PC, leftOver);
      }
    } while (!res.first);

    pc_graph.print_window_hash_map_freq();

    pc_graph.print_self_loop_info(&PC_SELF_LOOP_FILE);
    pc_graph.print_strongly_connected_components(&PC_SCC_FILE);
    pc_graph.print_value_frequency(&PC_ADDR_FREQ_FILE);

    mem_graph.print_self_loop_info(&MEM_SELF_LOOP_FILE);
    mem_graph.print_strongly_connected_components(&MEM_SCC_FILE);
    mem_graph.print_value_frequency(&MEM_ADDR_FREQ_FILE);
  }
}
