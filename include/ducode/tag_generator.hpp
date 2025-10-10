/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include "ducode/port.hpp"
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <gsl/narrow>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <cstdint>
#include <memory>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ducode {
class TagGenerator {
protected:
  //NOLINTBEGIN(*-non-private-member-variables-in-classes)
  uint32_t number_of_injected_tags = 0;
  std::vector<std::reference_wrapper<const Port> > m_ports;
  // supposed to only be input ports i guess? currently has to be all inputs so that the tagging works not in the taggenerator
  // the testbench has to track the tagged ports vs untagged ports
  std::shared_ptr<VCDFile> m_vcd_data;
  //NOLINTEND(*-non-private-member-variables-in-classes)

public:
  TagGenerator(const std::vector<std::reference_wrapper<const Port> > &ports,
               std::shared_ptr<VCDFile> vcd_data) : m_ports(ports), m_vcd_data(std::move(vcd_data)) {
  }

  virtual ~TagGenerator() = default;

  // Copy operations
  TagGenerator(const TagGenerator &other);

  TagGenerator &operator=(const TagGenerator &other);

  // Move operations
  TagGenerator(TagGenerator &&other) noexcept;

  TagGenerator &operator=(TagGenerator &&other) noexcept;

  virtual bool is_tagged(const Port &port, const VCDTime &timepoint) = 0;

  [[nodiscard]] uint32_t get_number_of_injected_tags() const {
    return number_of_injected_tags;
  }

  enum class type : uint8_t {
    FullResolution,
    Random
  };

  [[nodiscard]] virtual type get_type() const = 0;
};

class FullResolutionTagGenerator : public TagGenerator {
public:
  FullResolutionTagGenerator(const std::vector<std::reference_wrapper<const Port> > &ports,
                             std::shared_ptr<VCDFile> vcd_data) : TagGenerator(ports, std::move(vcd_data)) {
  }

  bool is_tagged(const Port & /*port*/, const VCDTime & /*timepoint*/) override {
    number_of_injected_tags++;
    return true;
  }

  [[nodiscard]] type get_type() const override {
    return type::FullResolution;
  }
};

class DeterministicRandomTagGenerator : public TagGenerator {
  static constexpr uint32_t max_coverage = 100;
  static constexpr uint32_t default_seed = 42;// Named constant for default seed
  uint32_t m_coverage;
  std::mt19937 m_gen;

  // Hash function for std::pair
  struct PairHash {
    template<class T1, class T2>
    size_t operator()(const std::pair<T1, T2> &p) const {
      auto hash1 = std::hash<T1>{}(p.first);
      auto hash2 = std::hash<T2>{}(p.second);
      return static_cast<size_t>(hash1) ^ (static_cast<size_t>(hash2) << 1u);
    }
  };

  // Store pre-computed results in a map indexed by port ID and timepoint
  std::unordered_map<std::pair<size_t, VCDTime>, bool, PairHash> m_results_cache;

public:
  DeterministicRandomTagGenerator(const std::vector<std::reference_wrapper<const Port> > &ports,
                                  std::shared_ptr<VCDFile> vcd_data,
                                  const uint32_t coverage,
                                  const uint32_t seed = default_seed) : TagGenerator(ports, std::move(vcd_data)),
                                                                        m_coverage(coverage),
                                                                        m_gen(seed)// Use a fixed seed for reproducibility
  {
    if (m_coverage > max_coverage) {
      throw std::runtime_error(fmt::format("The coverage max value is {}.", max_coverage));
    }

    // Pre-compute results for all ports and timepoints
    precompute_results();
  }

  void precompute_results() {
    std::discrete_distribution<> distribution({gsl::narrow<double>(m_coverage), gsl::narrow<double>(max_coverage - m_coverage)});

    std::vector<VCDTime> timepoints;

    if (m_vcd_data) {
      timepoints = m_vcd_data->get_timestamps();
    }

    // Generate results for each port at each timepoint
    for (const auto &port_ref: m_ports) {
      const Port &port = port_ref.get();
      size_t port_id = std::hash<Port>()(port);

      for (const auto &timepoint: timepoints) {
        // Generate a deterministic result based on port and timepoint
        bool is_tagged = (distribution(m_gen) == 0);
        m_results_cache[{port_id, timepoint}] = is_tagged;
      }
    }
  }

  // check if the given port is part of the tagged ports
  bool is_tagged(const Port &port, const VCDTime &timepoint) override {
    size_t port_id = std::hash<Port>()(port);
    auto key = std::make_pair(port_id, timepoint);

    // Check if we have a pre-computed result
    auto iter = m_results_cache.find(key);
    if (iter != m_results_cache.end()) {
      return iter->second;
    }

    // If not in cache (unusual case), compute it now and store it
    std::discrete_distribution<> distribution({gsl::narrow<double>(m_coverage), gsl::narrow<double>(max_coverage - m_coverage)});
    bool is_tagged = (distribution(m_gen) == 0);
    m_results_cache[key] = is_tagged;

    number_of_injected_tags++;

    return is_tagged;
  }

  [[nodiscard]] type get_type() const override {
    return type::Random;
  }
};
}// namespace ducode
