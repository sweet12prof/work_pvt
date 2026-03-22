#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>
#include <iostream>

#include "fileprocess.hpp"

#ifndef GLOBALANALYSER_H
#define GLOBALANALYSER_H

struct edgeProp {
  INT64 self_loop_count = 0;
};

struct Vertex_Data {
  ADDRINT value;
  UINT64 value_frequency;
};

enum Vertex_Property { MEM_ADDRESS, PC };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                    Vertex_Data, edgeProp>;

UINT64 extract_vertex_from_trace(std::vector<TraceData>& traceChunk,
                                 UINT64 trace_index, Vertex_Property property);

class GlobalAnalyser_FT {
 public:
  void fill_graph(std::vector<TraceData>& traceChunk,
                  Vertex_Property vertex_property);
  void generate_strongly_connected_componenets();

  void print_self_loop_info(std::ostream* out) const;
  void print_value_frequency(std::ostream* out) const;
  void print_strongly_connected_components(std::ostream* out) const;

 private:
  Graph g;
  // address to vertex ID map
  std::pair<INT64, std::vector<INT64>> component_result_pair;
  std::map<UINT64, Graph::vertex_descriptor> value_to_vertex;
};

#endif