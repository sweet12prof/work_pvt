#include <algorithm>
#include <analyser.hpp>
#include <cassert>
#include <cmath>
#include <iomanip>
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
      per_pc_dist.insert({item.pc,          // pc
                          {1,               // dynamic_inst/pc count
                           item.memAddress, // last mem_address
                           item.memAddress,
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
    (*out) << "\tZero Stride Ratio: " << item.second.metrics.zero_stride_ratio
           << std::endl
           << "\tMean: " << item.second.metrics.mean << std::endl
           << "\tVariance: " << item.second.metrics.variance << std::endl
           << std::endl
           << "\tNormalized Entropy: " << std::fixed << std::setprecision(6)
           << item.second.metrics.entropy << std::endl
           << "\tSign Symmetry: " << item.second.metrics.sign_symmetry_ratio
           << std::endl
           << std::endl;
    for (auto item2 : item.second.stride_freq)
      (*out) << "\tStride: " << item2.first << " Frequency: " << item2.second
             << std::endl;
  }
}

void Address_Analysis::Analyser::gen_classifiction_metrics() {
  // for (auto pc_dist : per_pc_dist{auto};)
  get_per_pc_dominant_stride_ratio();
  get_per_pc_zero_stride_ratio();
  get_per_pc_mean();
  get_per_pc_variance();
  get_per_pc_normalized_entropy();
  get_per_pc_stride_symmetry();
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
  assert(freq_vec.size() != 0);
  if (freq_vec.size() <= 1)
    return 0.0;

  double entropy = 0.0;
  for (UINT64 freq : freq_vec) {
    assert(freq != 0);
    assert(dynamic_load_count != 0);
    double p = static_cast<double>(freq) / dynamic_load_count;
    entropy += p * std::log2(p);
  }
  return -entropy / std::log2(freq_vec.size());
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

void Address_Analysis::Analyser::get_per_pc_zero_stride_ratio() {
  for (auto &pc_dist : per_pc_dist) {
    auto it = pc_dist.second.stride_freq.find(0);
    if (it == pc_dist.second.stride_freq.end()) {
      pc_dist.second.metrics.zero_stride_ratio = 0;
    } else {
      pc_dist.second.metrics.zero_stride_ratio =
          Address_Analysis::get_zero_stride_ratio(
              it->second, pc_dist.second.dynamic_pc_count);
    }
  }
}

void Address_Analysis::Analyser::get_per_pc_mean() {
  for (auto &pc_dist : per_pc_dist) {
    std::vector<std::pair<INT64, UINT64>> stride_freq_pair;
    if (pc_dist.second.stride_freq.size() < METRIC_RATIO) {
      pc_dist.second.metrics.mean = 0;
    } else {
      for (auto item : pc_dist.second.stride_freq) {
        if (item.first == pc_dist.second.first_memaddress) {
          continue;
        }
        stride_freq_pair.push_back({item.first, item.second});
      }
      pc_dist.second.metrics.mean = Address_Analysis::get_mean(
          stride_freq_pair, (pc_dist.second.dynamic_pc_count - 1));
    }
  }
}

void Address_Analysis::Analyser::get_per_pc_variance() {
  for (auto &pc_dist : per_pc_dist) {
    std::vector<std::pair<INT64, UINT64>> stride_freq_pair;
    if (pc_dist.second.stride_freq.size() < METRIC_RATIO) {
      pc_dist.second.metrics.variance = 0;
    } else {
      for (auto item : pc_dist.second.stride_freq) {
        if (item.first == pc_dist.second.first_memaddress) {
          continue;
        }
        stride_freq_pair.push_back({item.first, item.second});
      }
      pc_dist.second.metrics.variance = Address_Analysis::get_variance(
          stride_freq_pair, pc_dist.second.metrics.mean,
          (pc_dist.second.dynamic_pc_count - 1));
    }
  }
}

void Address_Analysis::Analyser::get_per_pc_normalized_entropy() {
  for (auto &item : per_pc_dist) {
    auto stride_vec_per_pc =
        Address_Analysis::get_per_stride_freq_vec(item.second.stride_freq);
    item.second.metrics.entropy = Address_Analysis::get_normalized_entropy(
        stride_vec_per_pc, item.second.dynamic_pc_count);
  }
}

double Address_Analysis::get_sign_symmetry_ratio(
    const std::vector<INT64> &stride_vec) {
  UINT positive_samples{0}, negative_samples{0};
  for (auto item : stride_vec)
    if (item < 0)
      negative_samples++;
    else
      positive_samples++;
  return (negative_samples == 0) ? 100000
                                 : (double)positive_samples / negative_samples;
}

std::vector<INT64> Address_Analysis::get_strides_per_pc(
    const std::map<INT64, UINT64> &stride_freq_map) {
  std::vector<INT64> stride_vec;
  for (auto item : stride_freq_map)
    stride_vec.push_back(item.first);
  return stride_vec;
}

void Address_Analysis::Analyser::get_per_pc_stride_symmetry() {
  for (auto &item : per_pc_dist) {
    auto strides_per_pc_vec =
        Address_Analysis::get_strides_per_pc(item.second.stride_freq);
    item.second.metrics.sign_symmetry_ratio =
        Address_Analysis::get_sign_symmetry_ratio(strides_per_pc_vec);
  }
}

/*
/opt/pin-3.31/source/include/pin
/opt/pin-3.31/source/include/pin/gen
/opt/pin-3.31/source/tools/InstLib
/opt/pin-3.31/extras/xed-intel64/include/xed
/opt/pin-3.31/extras/components/include
/opt/pin-3.31/extras/cxx/include
/opt/pin-3.31/extras
/opt/pin-3.31/extras/crt/include
/opt/pin-3.31/extras/crt
/opt/pin-3.31/extras/crt/include/arch-x86_64
/opt/pin-3.31/extras/crt/include/kernel/uapi
/opt/pin-3.31/extras/crt/include/kernel/uapi/asm-x86
*/