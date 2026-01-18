#include <algorithm>
#include <analyser.hpp>
#include <cassert>
#include <cmath>
#include <numeric>
#include <utils.hpp>

Address_Analysis::Analyser::Analyser(std::istream *file)
    : file(file), traceChunk(readFile(file)), loads_processed(0),
      accesses_count(0) {}

void Address_Analysis::Analyser::gen_pc_stide_distribution() {
  if (traceChunk.second.empty())
    return;
  for (auto item : traceChunk.second) {
    accesses_count++;
    if (item.inst_type != InstType::MEM_LOAD)
      continue;
    // check if the struct has ever being seen
    auto p{per_pc_dist.find(item.pc)};
    if (p == per_pc_dist.end()) {
      std::map<INT64, UINT64> stride_freq_dist;
      stride_freq_dist[item.memAddress] = 1;
      per_pc_dist.insert({item.pc,                 // pc
                          {1,                      // dynamic_inst/pc count
                           item.memAddress,        // last mem_address
                           (INT64)item.memAddress, // init stride ini
                           {},
                           stride_freq_dist}});

    } else {
      (p->second.dynamic_pc_count)++;
      INT64 new_stride = item.memAddress - (p->second.last_memaddress);
      p->second.stride_freq[new_stride]++;
      p->second.last_memaddress = item.memAddress;
    }
    loads_processed++;
  }
}

void Address_Analysis::Analyser::process_trace() {
  do {
    gen_pc_stide_distribution();
    traceChunk = readFile(file);
  } while (!traceChunk.first);

}

void Address_Analysis::Analyser::print_per_pc_stride_dist(std::ostream *out) {
  (*out) << " Loads processed: " << loads_processed << std::endl;
  (*out) << "accesses processed: " << accesses_count << std::endl
         << std::endl
         << std::endl;
  for (auto item : per_pc_dist) {
    (*out) << "pc :  " << item.first << std::endl;
    (*out) << "\tDynamic count of static load: " << item.second.dynamic_pc_count
           << std::endl;
    (*out) << "\tDominant Stride Ratio: "
           << item.second.metrics.dominant_stride_ratio << std::endl;
    for (auto item2 : item.second.stride_freq)
      (*out) << "\tStride: " << item2.first << " Frequency: " << item2.second
             << std::endl;
  }
}

void Address_Analysis::Analyser::gen_classifiction_metrics() {
  // for (auto pc_dist : per_pc_dist{auto};)
  get_per_pc_dominant_stride_ratio();
}

double
Address_Analysis::get_dominant_stride_ratio(const std::vector<UINT64> &freq_vec,
                                            UINT64 dynamic_load_count) {
  auto it = std::max_element(freq_vec.begin(), freq_vec.end());
  return dynamic_load_count < STRIDE_RATIO_LOWER_THRESHOLD
             ? 0
             : static_cast<double>(*it) / dynamic_load_count;
}

double Address_Analysis::get_mean(
    const std::vector<std::pair<INT64, UINT64>> &freq_map,
    UINT64 dynamic_load_count) {
  double sum = std::accumulate(
      freq_map.begin(), freq_map.end(), 0.0,
      [](double sum_val, const std::pair<INT64, UINT64> &unit_freq_map) {
        return sum_val +
               static_cast<double>(unit_freq_map.first) * unit_freq_map.second;
      });
  return sum / dynamic_load_count;
}

double Address_Analysis::get_variance(
    const std::vector<std::pair<INT64, UINT64>> &freq_vec, double mean,
    UINT64 dynamic_load_count) {
  auto sum = std::accumulate(
      freq_vec.begin(), freq_vec.end(), 0.0,
      [mean](double sum_val, const std::pair<INT64, UINT64> &unit_freq_map) {
        return sum_val + unit_freq_map.second *
                             std::pow(((double)unit_freq_map.first - mean), 2);
      });

  return sum / dynamic_load_count;
}

double
Address_Analysis::get_normalized_entropy(const std::vector<UINT64> &freq_vec,
                                         UINT64 dynamic_load_count) {
  std::vector<double> prob_vec(freq_vec.size());

  std::transform(freq_vec.begin(), freq_vec.end(), prob_vec.begin(),
                 [dynamic_load_count](UINT64 freq) {
                   double p = (double)freq / dynamic_load_count;
                   assert(
                       p != 0 &&
                       "this should never happen, probability cannot be zero");
                   return p;
                 });

  auto entropy_accum = std::accumulate(
      prob_vec.begin(), prob_vec.end(), 0.0, [](double sum, double prob) {
        return sum + static_cast<double>(prob) * std::log2(prob);
      });
  return -entropy_accum / (std::log2(freq_vec.size()));
}

std::vector<UINT64> Address_Analysis::get_per_stride_freq_vec(
    const std::map<INT64, UINT64> &stride_map) {
  std::vector<UINT64> freq_vec;
  for (auto item : stride_map) {
    freq_vec.push_back(item.second);
  }
  return freq_vec;
}

void Address_Analysis::Analyser::get_per_pc_dominant_stride_ratio() {
  for (auto &item : per_pc_dist) {
    auto stride_freq_vec =
        Address_Analysis::get_per_stride_freq_vec(item.second.stride_freq);
    auto dom_stride_ratio = get_dominant_stride_ratio(
        stride_freq_vec, item.second.dynamic_pc_count);
    // assert(dom_stride_ratio != 0 && "ratio cannot be zero");
    item.second.metrics.dominant_stride_ratio = dom_stride_ratio;
  }
}
