#include "GlobalAnalyser.hpp"

#include <cassert>
/*
      Implementation
*/

void GlobalAnalyser_FT::fill_graph(std::vector<TraceData>& traceChunk,
                                   Vertex_Property vertex_property) {
  for (UINT64 i{0}; i < traceChunk.size(); i++) {
    auto value = extract_vertex_from_trace(traceChunk, i, vertex_property);
    auto& value_index = value_to_vertex_table.get<addrint_tag>();
    auto it = value_index.find(value);
    if (it == value_index.end()) {
      auto vertex = boost::add_vertex({value, 1}, g);
      value_to_vertex_table.insert({vertex, value});
    } else {
      g[it->vertex_id].value_frequency++;
    }
  }

  for (size_t i = 0; i + 1 < traceChunk.size(); ++i) {
    auto curr_property =
        extract_vertex_from_trace(traceChunk, i, vertex_property);
    auto next_property =
        extract_vertex_from_trace(traceChunk, i + 1, vertex_property);

    auto& value_index = value_to_vertex_table.get<addrint_tag>();
    auto curr_vertex = value_index.find(curr_property);
    auto next_vertex = value_index.find(next_property);

    assert(curr_vertex != value_index.end() &&
           next_vertex != value_index.end() &&
           "All vertices and their corresponding value must be in table");

    auto [e, exists] =
        boost::edge(curr_vertex->vertex_id, next_vertex->vertex_id, g);

    if (!exists) {
      bool inserted;
      boost::tie(e, inserted) =
          boost::add_edge(curr_vertex->vertex_id, next_vertex->vertex_id, g);
      g[e].self_loop_count = 0;
    }

    // THIS IS THE IMPORTANT PART
    if (curr_vertex == next_vertex) {
      g[e].self_loop_count++;
    }
  }
}

void GlobalAnalyser_FT::print_self_loop_info(std::ostream* out) const {
  for (auto item : boost::make_iterator_range(edges(g))) {
    (*out) << (void*)g[boost::source(item, g)].value << "->"
           << (void*)g[boost::target(item, g)].value << "\t:" << "["
           << g[item].self_loop_count << "]" << std::endl;
  }
}

void GlobalAnalyser_FT::generate_strongly_connected_componenets() {
  component_result_pair.second.resize(boost::num_vertices(g));

  component_result_pair.first = boost::strong_components(
      g, boost::make_iterator_property_map(component_result_pair.second.begin(),
                                           boost::get(boost::vertex_index, g)));
}

void GlobalAnalyser_FT::print_strongly_connected_components(
    std::ostream* out) const {
  (*out) << "Number of SCCs " << component_result_pair.first << std::endl;

  for (int i = 0; i < component_result_pair.second.size(); ++i) {
    (*out) << "Vertex " << i << " is in component "
           << component_result_pair.second[i] << std::endl;
  }
}

void GlobalAnalyser_FT::print_value_frequency(std::ostream* out) const {
  for (auto item : boost::make_iterator_range(vertices(g))) {
    (*out) << "Vertex: " << (void*)g[item].value
           << " Frequency: " << g[item].value_frequency << std::endl;
  }
}

UINT64 extract_vertex_from_trace(std::vector<TraceData>& traceChunk,
                                 UINT64 trace_index, Vertex_Property property) {
  switch (property) {
    case Vertex_Property::PC:
      return traceChunk[trace_index].pc;
      break;

    case Vertex_Property::MEM_ADDRESS:
    default:
      return traceChunk[trace_index].memAddress;
      break;
  }
}

void generate_window_hashes(std::vector<TraceData>& traceChunk,
                            std::size_t window_length, UINT64& hash, std::) {}

UINT64 generate_initial_hash(std::vector<TraceData>& traceChunk,
                             std::size_t window_length,
                             Vertex_Property property) {
  UINT64 hash{0};
  for (std::size_t i{0}; i < window_length; i++) {
    auto value = extract_vertex_from_trace(traceChunk, i, property);
    hash += value * constants_table[i];
  }
  return hash;
}