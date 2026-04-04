#include <iostream>

#include "fileprocess.hpp"

#ifndef GLOBALANALYSER_H
#define GLOBALANALYSER_H

UINT64 extract_vertex_from_trace(std::vector<TraceData>& traceChunk,
                                 UINT64 trace_index, Vertex_Property property);

// sliding Window Hash
UINT64 generate_window_hashes(std::vector<TraceData>& traceChunk,
                              std::size_t window_length);
UINT64 generate_initial_hash(std::vector<TraceData>& traceChunk,
                             std::size_t window_length);

class GlobalAnalyser_FT {
 public:
  void fill_graph(std::vector<TraceData>& traceChunk,
                  Vertex_Property vertex_property);
  void generate_strongly_connected_componenets();

  /*              VALIDATE SLIDING WINDOWS                  */
  // returns true if all elements in window are members of the same SCC
  bool check_window_element_scc_membership(std::vector<UINT64>& window);
  // returns true if window elements ordering have valid graph transitions
  bool validate_window_element_transitions(std::vector<UINT64>& window);

  /*      GENERATE AND PRE-FILTER WINDOWS, FREQUENCY         */
  std::vector<std::vector<UINT64>> generate_sliding_windows();

  void print_self_loop_info(std::ostream* out) const;
  void print_value_frequency(std::ostream* out) const;
  void print_strongly_connected_components(std::ostream* out) const;

 private:
  Graph g;
  // address to vertex ID map
  std::pair<INT64, std::vector<INT64>> component_result_pair;
  Value_To_Vertex_Container value_to_vertex_table;
};

#endif