#include <deque>
#include <iostream>
#include <map>

#include "fileprocess.hpp"

#ifndef GLOBALANALYSER_H
#define GLOBALANALYSER_H

struct window_info {
  UINT64 frequency;
  ADDRINT first_element;
};

UINT64 extract_value_from_trace(std::vector<TraceData>& traceChunk,
                                UINT64 trace_index, Vertex_Property property);

// // sliding Window Hash
// std::vector<std::vector<TraceData>> generate_windows(
//     std::vector<TraceData>& traceChunk, std::vector<TraceData>& leftOver,
//     size_t window_size);

inline UINT64 generate_initial_hash(std::vector<TraceData>& traceChunk,
                                    std::size_t window_length,
                                    Vertex_Property property) {
  UINT64 hash{0};
  for (std::size_t i{0}; i < window_length; i++) {
    auto value = extract_value_from_trace(traceChunk, i, property);
    hash += value * constants_table[i];
  }
  return hash;
}

class GlobalAnalyser_FT {
 public:
  void fill_graph(std::vector<TraceData>& traceChunk,
                  Vertex_Property vertex_property);

  void add_single_transition(TraceData& curr_element, TraceData& next_element,
                             Vertex_Property property);

  void generate_strongly_connected_componenets();

  /*              VALIDATE SLIDING WINDOWS                  */
  // returns true if all elements in window are members of the same SCC
  bool check_window_element_scc_membership(std::vector<UINT64>& window);
  // returns true if window elements ordering have valid graph transitions
  bool validate_window_element_transitions(std::vector<UINT64>& window);

  /*      GENERATE AND PRE-FILTER WINDOWS, FREQUENCY         */
  std::vector<std::vector<UINT64>> generate_sliding_windows();

  // void generate_window_distribution(std::vector<TraceData>& traceChunk,
  //                                   size_t window_size);

  void print_self_loop_info(std::ostream* out) const;
  void print_value_frequency(std::ostream* out) const;
  void print_strongly_connected_components(std::ostream* out) const;

  void print_window_hash_map_freq() const {
    for (auto item : window_freq_map) {
      std::cout << "Hash: " << item.first
                << " Frequency: " << item.second.frequency << std::endl;
    }
  }

  bool is_window_in_scc(std::deque<ADDRINT>& window_sequence) {
    if (window_sequence.empty()) return false;
    auto& value_index = value_to_vertex_table.get<addrint_tag>();
    auto it = value_index.find(window_sequence[0]);
    assert(it != value_index.end() &&
           "All elements must be in value_to_vertex_table");
    auto first_component = component_result_pair.second[it->vertex_id];
    // if element does not recur no need to consider window
    if (g[it->vertex_id].value_frequency == 1) return false;
    for (std::size_t i{1}; i < window_sequence.size(); i++) {
      it = value_index.find(window_sequence[i]);
      assert(it != value_index.end() &&
             "All elements must be in value_to_vertex_table");
      // if element does not recur no need to consider window
      if (g[it->vertex_id].value_frequency == 1) return false;
      if (component_result_pair.second[it->vertex_id] != first_component)
        return false;
    }
    return true;
  };

  void hash_windows(std::vector<TraceData>& traceChunk,
                    std::size_t window_length, Vertex_Property property,
                    std::vector<TraceData>& leftOver);

 private:
  Graph g;
  // address to vertex ID map
  std::pair<INT64, std::vector<INT64>> component_result_pair;
  Value_To_Vertex_Container value_to_vertex_table;
  std::map<UINT64, window_info> window_freq_map;
};

#endif