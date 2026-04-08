#include "GlobalAnalyser.hpp"

#include <cassert>
/*
      Implementation
*/

void GlobalAnalyser_FT::fill_graph(std::vector<TraceData>& traceChunk,
                                   Vertex_Property vertex_property) {
  for (UINT64 i{0}; i < traceChunk.size(); i++) {
    auto value = extract_value_from_trace(traceChunk, i, vertex_property);
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
        extract_value_from_trace(traceChunk, i, vertex_property);
    auto next_property =
        extract_value_from_trace(traceChunk, i + 1, vertex_property);

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
           << (void*)g[boost::target(item, g)].value << "\t:"
           << "[" << g[item].self_loop_count << "]" << std::endl;
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

UINT64 extract_value_from_trace(std::vector<TraceData>& traceChunk,
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

void GlobalAnalyser_FT::hash_windows(std::vector<TraceData>& traceChunk,
                                     std::size_t window_length,
                                     Vertex_Property property,
                                     std::vector<TraceData>& leftOver) {
  std::deque<ADDRINT> window;
  for (size_t i{0}; i < window_length; i++) {
    window.push_back(extract_value_from_trace(traceChunk, i, property));
  }
  // generate hash for first window
  auto hash = generate_initial_hash(traceChunk, window_length, property);
  // check if first window is an SCC member
  auto is_member = GlobalAnalyser_FT::is_window_in_scc(window);
  if (is_member) {
    window_freq_map[hash].frequency++;
    window_freq_map[hash].first_element = window.front();
  }

  UINT64 first_term{1};
  for (size_t i{0}; i < window_length - 1; i++) first_term *= POLY_BASE;

  for (size_t i{0}; i < traceChunk.size() - window_length; i++) {
    // get property; mem address or pc
    if (i + window_length >= traceChunk.size()) break;
    auto new_window_element =
        extract_value_from_trace(traceChunk, i + window_length, property);

    // generate new hash
    hash -= window.front() * first_term;
    // get next window
    window.pop_front();
    window.push_back(new_window_element);
    hash *= POLY_BASE;
    hash += new_window_element;

    is_member = is_window_in_scc(window);
    if (is_member) {
      auto& entry = window_freq_map[hash];
      if (entry.frequency == 0) entry.first_element = window.front();
      // assert(window_freq_map[hash].first_element == window.front() &&
      //        "COLLISION OCCURED");
      entry.frequency++;
    }
  }
  size_t remainder = (traceChunk.size() < window_length) ? traceChunk.size()
                                                         : window_length - 1;
  leftOver.clear();
  if (remainder > 0) {
    leftOver.insert(leftOver.end(), (traceChunk.end() - remainder),
                    traceChunk.end());
  }
}

void GlobalAnalyser_FT::add_single_transition(TraceData& curr_element,
                                              TraceData& next_element,
                                              Vertex_Property property) {
  auto curr_value =
      property == MEM_ADDRESS ? curr_element.memAddress : curr_element.pc;

  auto next_value =
      property == MEM_ADDRESS ? next_element.memAddress : next_element.pc;

  auto& value_index = value_to_vertex_table.get<addrint_tag>();
  auto next_vertex = value_index.find(next_value);
  Graph::vertex_descriptor vid;
  if (next_vertex == value_index.end()) {
    vid = boost::add_vertex({next_value, 1}, g);
    value_to_vertex_table.insert({vid, next_value});
  } else {
    g[next_vertex->vertex_id].value_frequency++;
    vid = next_vertex->vertex_id;
  }

  auto curr_vertex = value_index.find(curr_value);

  assert(curr_vertex != value_index.end() &&
         "All vertices and their corresponding value must be in table");

  auto [e, exists] = boost::edge(curr_vertex->vertex_id, vid, g);

  if (!exists) {
    bool inserted;
    boost::tie(e, inserted) = boost::add_edge(curr_vertex->vertex_id, vid, g);
    g[e].self_loop_count = 0;
  }

  // THIS IS THE IMPORTANT PART
  if (curr_vertex->vertex_id == vid) {
    g[e].self_loop_count++;
  }
}
