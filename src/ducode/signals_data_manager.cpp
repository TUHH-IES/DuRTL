#include <ducode/ids.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/utility/legalize_identifier.hpp>
#include <ducode/utility/smt.hpp>

#include <boost/algorithm/string/join.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>// NOLINT(misc-include-cleaner)
#include <gsl/assert>
#include <gsl/narrow>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>// NOLINT(misc-include-cleaner)
#include <vcd-parser/VCDTimedValue.hpp>
#include <vcd-parser/VCDTypes.hpp>
#include <vcd-parser/VCDValue.hpp>
#include <z3++.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ducode {

namespace {
inline uint32_t value_vector_to_uint32(const VCDBitVector& value_vector) {
  uint32_t result = 0;
  const uint32_t max_size = 32;
  if (value_vector.size() > max_size) { throw std::runtime_error("VCDBitVector is too large for uint32_t"); }
  for (uint64_t i = 0; i < value_vector.size(); ++i) {
    if (value_vector[i] == VCDBit::VCD_X || value_vector[i] == VCDBit::VCD_Z) {
      throw std::runtime_error("VCDBitVector contains X or Z values!");
    }
    if (value_vector[i] == VCDBit::VCD_1) {
      result |= (1u << ((value_vector.size() - 1) - i));
    }
  }
  return result;
}
}// namespace

[[nodiscard]] std::unordered_map<SignalIdentifier, VCDSignal*> VCDSignalsDataManager::get_source_tag_signals(uint64_t index, const std::vector<SignalIdentifier>& source_signals) const {
  std::unordered_map<SignalIdentifier, VCDSignal*> input_to_tag_signals;
  auto all_signals = m_root_scopes[index]->signals;
  for (const auto& input_id: source_signals) {
    for (const auto& vcd_signal: all_signals) {
      if (vcd_signal->reference == fmt::format("{}_t", input_id.name)) {
        input_to_tag_signals[input_id] = vcd_signal;
      }
    }
  }
  return input_to_tag_signals;
}

[[nodiscard]] TagSourceList VCDSignalsDataManager::get_information_flow_source_signals_for_target_signal_at_timestep(const std::vector<SignalIdentifier>& source_signals, const SignalIdentifier& target_signal, SignalTime timestep) const {
  TagSourceList decoded_tag_list;
  IFInfo decoded_tag;

  for (const auto& [index, vcd_data]: std::ranges::views::enumerate(m_vcd_data)) {

    // get the tag value for the output at timestep
    uint32_t tag_value = 0;
    auto* output_signal_ptr = get_vcd_tag_signal_ptr(target_signal, 0);
    auto tags = m_vcd_data[gsl::narrow<uint64_t>(index)]->get_signal_values(output_signal_ptr->hash);
    for (const auto& tag: tags) {
      if (tag.time == timestep) {
        tag_value = value_vector_to_uint32(tag.value.get_value_vector());
        break;
      }
    }

    if (tag_value == 0) {
      // tag_value == 0 => No tags found for the target signal at the specified timestep
      continue;
    }

    // Get the vcd signals for the inputs
    std::unordered_map<SignalIdentifier, VCDSignal*> input_to_tag_signals = get_source_tag_signals(gsl::narrow<uint64_t>(index), source_signals);

    // use bitwise and to check if the input tags are contained in the output tag value
    for (const auto& input_tag_signal: input_to_tag_signals) {
      for (const auto& signal_value: vcd_data->get_signal_values(input_tag_signal.second->hash)) {
        auto val = value_vector_to_uint32(signal_value.value.get_value_vector());
        if ((val & tag_value) > 0) {
          decoded_tag.tag_injection_time = gsl::narrow<uint32_t>(signal_value.time);
          decoded_tag.input_id = input_tag_signal.first;
          decoded_tag.tag_observation_time = timestep;
          decoded_tag_list.m_tag_vector.emplace_back(decoded_tag);
        }
      }
    }
  }
  return decoded_tag_list;
}

// goes through all vcd files; gets the tag value for the target signal at each timestep in its deque<TimedSignalValue>;
// then checks if the tag value contains any of the input tags; if so, it adds the tag to the decoded_tag_list
[[nodiscard]] TagSourceList VCDSignalsDataManager::get_information_flow_source_signals_for_target_signal(const std::vector<SignalIdentifier>& source_signals, const SignalIdentifier& target_signal) const {
  TagSourceList decoded_tag_list;
  IFInfo decoded_tag;

  for (const auto& [index, vcd_data]: std::ranges::views::enumerate(m_vcd_data)) {

    // Get the vcd signals for the inputs
    std::unordered_map<SignalIdentifier, VCDSignal*> input_to_tag_signals = get_source_tag_signals(gsl::narrow<uint32_t>(index), source_signals);

    // get the tag value for the output at timestep
    uint32_t tag_value = 0;
    auto* output_signal_ptr = get_vcd_tag_signal_ptr(target_signal, 0);
    auto tags = m_vcd_data[gsl::narrow<uint32_t>(index)]->get_signal_values(output_signal_ptr->hash);
    for (const auto& timed_tag: tags) {
      tag_value = value_vector_to_uint32(timed_tag.value.get_value_vector());
      // use bitwise and to check if the input tags are contained in the output tag value
      for (const auto& input_tag_signal: input_to_tag_signals) {
        for (const auto& signal_value: vcd_data->get_signal_values(input_tag_signal.second->hash)) {
          auto val = value_vector_to_uint32(signal_value.value.get_value_vector());
          if ((val & tag_value) > 0) {
            decoded_tag.tag_injection_time = gsl::narrow<uint32_t>(signal_value.time);
            decoded_tag.input_id = input_tag_signal.first;
            decoded_tag.tag_observation_time = gsl::narrow<uint32_t>(timed_tag.time);
            decoded_tag_list.m_tag_vector.emplace_back(decoded_tag);
          }
        }
      }
    }
  }
  return decoded_tag_list;
}

SignalValues VCDSignalsDataManager::get_signal_values(const SignalIdentifier& signal_id) const {
  if (m_vcd_data.empty()) {
    throw std::runtime_error("the instantiation graph does not contain VCD data");
  }

  SignalValues values;
  for (const auto& signal_value: m_vcd_data[0]->get_signal_values(get_vcd_signal_ptr({.name = signal_id.name, .hierarchy = signal_id.hierarchy})->hash)) {
    SignalValue value(signal_value.value);
    values.m_timed_signal_values.emplace_back(TimedSignalValue(signal_value.time, value));
  }

  return values;
}

[[nodiscard]] uint64_t VCDSignalsDataManager::get_signal_size(const SignalIdentifier& signal) const {
  return gsl::narrow<uint64_t>(get_signal(signal).size);
}

[[nodiscard]] uint64_t SMTSignalsDataManager::get_signal_size(const SignalIdentifier& /*signal*/) const {
  throw std::runtime_error("not implemented");
}

[[nodiscard]] std::unordered_map<std::string, SignalValues> VCDSignalsDataManager::get_input_values(std::vector<SignalIdentifier> input_ids) const {
  std::unordered_map<std::string, SignalValues> signal_values;
  for (const auto& input_id: input_ids) {
    const SignalInfo input = get_signal(input_id);
    VCDSignalValues vcd_values = m_vcd_data[0]->get_signal_values(input.hash);
    signal_values[input.hash] = SignalValues{vcd_values};
  }
  return signal_values;
}

[[nodiscard]] SignalValue VCDSignalsDataManager::get_signal_value(const SignalIdentifier& signal_id, SignalTime timestep) const {
  const SignalInfo signal = get_signal(signal_id);
  VCDValue vcd_value = m_vcd_data[0]->get_signal_value_at(signal.hash, timestep);
  return SignalValue(vcd_value);
}

// call only if solver was SAT before
[[nodiscard]] std::unordered_map<std::string, SignalValues> SMTSignalsDataManager::get_input_values(std::vector<SignalIdentifier> input_ids) const {

  std::unordered_map<std::string, SignalValues> input_values;
  z3::model s_model = m_solver->get_model();
  for (auto& signal: input_ids) {
    try {
      input_values[fmt::format("{}_{}", boost::algorithm::join(signal.hierarchy, "_"), signal.name)] = get_signal_values(signal);
    } catch (z3::exception& e) {
      spdlog::warn("SMT solver does not provide input value for {}", signal.name);
      throw std::runtime_error(fmt::format("Z3 Exception: {}", e.msg()));
    }
  }
  return input_values;
}

[[nodiscard]] std::vector<SignalInfo> VCDSignalsDataManager::get_signals() const {
  std::vector<SignalInfo> signals;
  for (const VCDScope* scope: m_vcd_data[0]->get_scopes().at(0).children) {
    for (VCDSignal* signal: scope->signals) {
      signals.emplace_back(*signal);
    }
  }
  return signals;
}

[[nodiscard]] const std::vector<SignalTime>& VCDSignalsDataManager::get_timestamps() const {
  if (m_vcd_data.empty()) {
    throw std::runtime_error("No VCD data found");
  }
  return m_vcd_data[0]->get_timestamps();
}

[[nodiscard]] std::vector<SignalValues> VCDSignalsDataManager::get_tag_values(const SignalIdentifier& signal_id) const {
  std::vector<SignalValues> result;
  for (const auto& [index, vcd_data]: std::views::enumerate(m_vcd_data)) {
    auto uindex = gsl::narrow<uint64_t>(index);
    //todo: some edges do not have simulation values assigned to them
    if (!m_vcd_data.empty()) {
      result.emplace_back((SignalValues{(vcd_data->get_signal_values(get_vcd_tag_signal_ptr({.name = signal_id.name, .hierarchy = signal_id.hierarchy}, gsl::narrow<uint32_t>(uindex))->hash))}));
    }
  }
  return result;
}

[[nodiscard]] VCDSignal* VCDSignalsDataManager::get_vcd_signal_ptr(const SignalIdentifier& signal) const {
  const auto* top_module_scope = m_root_scopes[0];

  auto get_scope_hierarchy = [](const VCDScope& scope) -> std::vector<std::string> {
    std::deque<std::string> result;
    const VCDScope* temp = &scope;
    while (temp != nullptr) {
      result.emplace_front(temp->name);
      temp = temp->parent;
    }
    return {result.begin() + 2, result.end()};
  };

  auto compare_hierarchies = [](const auto& hierarchy1, const auto& hierarchy2) {
    auto drop1 = hierarchy1 | std::views::drop(1);
    auto drop2 = hierarchy2 | std::views::drop(1);

    return std::ranges::equal(drop1, drop2);
  };

  const VCDScope* selected_scope = nullptr;
  if (signal.hierarchy.size() == 1) {
    selected_scope = top_module_scope;
  } else {
    auto matching_scopes = m_vcd_data[0]->get_scopes() | std::views::filter([&](const auto& scope) { return scope.name == legalize_identifier(signal.hierarchy.back()); });
    auto matching_scopes_size = std::ranges::count_if(matching_scopes, [](const auto&) { return true; });
    if (matching_scopes_size == 1) {
      selected_scope = &*matching_scopes.begin();
    } else if (signal.hierarchy.size() > 2) {
      const auto scope_it = std::ranges::find_if(matching_scopes,
                                                 [&](const auto& scope) {
                                                   return compare_hierarchies(get_scope_hierarchy(scope), signal.hierarchy);
                                                 });
      if (scope_it == matching_scopes.end()) {
        spdlog::info("{}", signal.hierarchy);
        throw std::runtime_error("Scope not found.");
      }
      selected_scope = &*scope_it;
    } else {
      throw std::runtime_error("This is a strange case. Not managed for now.");
    }
  }

  auto net_name_legalized = legalize_identifier(signal.name);
  auto signal_it = std::ranges::find_if(selected_scope->signals, [&](const VCDSignal* find_signal) { return find_signal->reference == net_name_legalized; });
  if (signal_it == selected_scope->signals.end()) {
    throw std::runtime_error(fmt::format("The referenced net {} was not found in the scope {}", signal.name, selected_scope->name));
  }
  return *signal_it;
}

[[nodiscard]] VCDSignal* VCDSignalsDataManager::get_vcd_tag_signal_ptr(const SignalIdentifier& signal, const uint32_t& vcd_index) const {
  Expects(m_vcd_data[vcd_index]->root_scope != nullptr);
  Expects(!m_vcd_data[vcd_index]->root_scope->children.empty());
  auto* tb_scope = m_vcd_data[vcd_index]->root_scope->children[0];
  Expects(tb_scope != nullptr);
  Expects(!tb_scope->children.empty());
  auto* top_module_scope = *(tb_scope->children.begin());

  auto get_scope_hierarchy = [](const VCDScope& scope) -> std::vector<std::string> {
    std::deque<std::string> result;
    const VCDScope* temp = &scope;
    while (temp != nullptr) {
      result.emplace_front(temp->name);
      temp = temp->parent;
    }
    return {result.begin() + 2, result.end()};
  };

  auto compare_hierarchies = [](const auto& hierarchy1, const auto& hierarchy2) {
    auto drop1 = hierarchy1 | std::views::drop(1);
    auto drop2 = hierarchy2 | std::views::drop(1);

    return std::ranges::equal(drop1, drop2);
  };

  const VCDScope* selected_scope = nullptr;
  if (signal.hierarchy.size() == 1) {
    selected_scope = top_module_scope;
  } else {
    auto matching_scopes = m_vcd_data[vcd_index]->get_scopes() | std::views::filter([&](const auto& scope) { return scope.name == legalize_identifier(signal.hierarchy.back()); });
    auto matching_scopes_size = std::ranges::count_if(matching_scopes, [](const auto&) { return true; });
    if (matching_scopes_size == 1) {
      selected_scope = &*matching_scopes.begin();
    } else if (signal.hierarchy.size() > 2) {
      const auto scope_it = std::ranges::find_if(matching_scopes,
                                                 [&](const auto& scope) {
                                                   return compare_hierarchies(get_scope_hierarchy(scope), signal.hierarchy);
                                                 });
      if (scope_it == matching_scopes.end()) {
        spdlog::info("{}", signal.hierarchy);
        throw std::runtime_error("Scope not found.");
      }
      selected_scope = &*scope_it;
    } else {
      throw std::runtime_error("This is a strange case. Not managed for now.");
    }
  }

  auto net_name_legalized = legalize_identifier(fmt::format("{}_t", signal.name));
  auto signal_it = std::ranges::find_if(selected_scope->signals, [&](const VCDSignal* find_signal) { return find_signal->reference == net_name_legalized; });
  if (signal_it == selected_scope->signals.end()) {
    throw std::runtime_error(fmt::format("The referenced net {} was not found in the scope {}", signal.name, selected_scope->name));
  }
  return *signal_it;
}

bool VCDSignalsDataManager::bitwise_vcd_compare(VCDBitVector& vec1, VCDBitVector& vec2) const {
  if (vec1.size() != vec2.size()) {
    return false;
  }

  VCDBitVector result_vec;
  for (auto i = 0ul; i < vec1.size(); i++) {
    if (vec1[i] == vec2[i] && vec1[i] == VCDBit::VCD_1 && vec2[i] == VCDBit::VCD_1) {
      result_vec.emplace_back(VCDBit::VCD_1);
    } else {
      result_vec.emplace_back(VCDBit::VCD_0);
    }
  }
  return std::ranges::any_of(result_vec, [](auto bit) { return bit == VCDBit::VCD_1; });
}

[[nodiscard]] inline uint32_t getTagPattern(const size_t& size, const std::tuple<VCDTime, VCDBit>& value) {
  const auto& [index, tag_bit] = value;
  if (tag_bit == VCDBit::VCD_1) {
    auto uindex = gsl::narrow<uint32_t>(index);
    auto bitPosition = gsl::narrow<uint32_t>(size - uindex - 1);
    uint32_t bitPattern = static_cast<uint32_t>(1) << bitPosition;
    //GF temp - remove - just check rewrite of old code:
    //assert(bitPosition< sizeof()*8);
    assert(bitPattern == static_cast<unsigned int>(pow(2, gsl::narrow<double>(bitPosition))));
    assert(static_cast<unsigned int>(pow(2, gsl::narrow<double>(bitPosition))) == gsl::narrow<unsigned int>(static_cast<uint32_t>(1) << bitPosition));
    return bitPattern;
  }
  return 0;//Is that right?!
}

inline void insertTagUpdateCount(std::set<uint32_t>& tags, const size_t& size, const std::tuple<VCDTime, VCDBit>& value, uint32_t& tag_cnt) {
  const auto& [index, tag_bit] = value;
  uint32_t bitPattern = getTagPattern(size, value);
  if (bitPattern != 0) {
    if (!tags.contains(bitPattern)) {
      tags.insert(bitPattern);
      tag_cnt++;
    }
  }
}

[[nodiscard]] uint32_t VCDSignalsDataManager::get_tag_count_signal(const std::string& signal_name, const std::vector<std::string>& signal_hierarchy) const {
  auto* signal_vcd_ptr = get_vcd_signal_ptr(SignalIdentifier{.name = signal_name, .hierarchy = signal_hierarchy});

  uint32_t tag_cnt = 0;
  std::set<uint32_t> tags;
  std::vector<const VCDSignalValues*> result;
  for (const auto& [vcd_index, vcd_data]: std::views::enumerate(m_vcd_data)) {
    tags.clear();
    result.clear();
    // some edges do not have simulation values assigned to them
    if (signal_vcd_ptr != nullptr) {
      result.emplace_back(&(vcd_data->get_signal_values((signal_vcd_ptr)->hash)));
    }
    for (const auto* tag_vector: result) {
      for (const auto& tag_value: *tag_vector) {
        auto value_vector = tag_value.value.get_value_vector();
        //for (const auto& [index, tag_bit]: std::views::enumerate(value_vector)) {
        for (const auto& value: std::views::enumerate(value_vector)) {
          insertTagUpdateCount(tags, tag_value.value.get_value_vector().size(), value, tag_cnt);
        }
      }
    }
  }
  return tag_cnt;
}

[[nodiscard]] uint32_t VCDSignalsDataManager::get_tag_count_signal_to_signal(const SignalIdentifier& source_signal, const SignalIdentifier& target_signal) const {
  auto* source_signal_vcd_ptr = get_vcd_tag_signal_ptr(source_signal, 0);
  auto* target_signal_vcd_ptr = get_vcd_tag_signal_ptr(target_signal, 0);

  uint32_t tag_cnt = 0;
  std::set<uint32_t> tags;
  std::vector<std::set<uint32_t>> tags_ci_inputs;
  std::vector<const VCDSignalValues*> result;
  // collect all tags for the source signal from all VCD files
  for (const auto& [vcd_index, vcd_data]: std::views::enumerate(m_vcd_data)) {
    tags.clear();
    result.clear();
    // some edges do not have simulation values assigned to them
    if (source_signal_vcd_ptr != nullptr)// m_graph[*edge_vcd_ptr].tags_signal.empty())
    {
      result.emplace_back(&(vcd_data->get_signal_values(source_signal_vcd_ptr->hash)));
    }

    for (const auto* tag_vector: result) {
      for (const auto& tag_value: *tag_vector) {
        auto value_vector = tag_value.value.get_value_vector();
        //for (const auto& [index, tag_bit]: std::views::enumerate(value_vector)) {
        for (const auto& value: std::views::enumerate(value_vector)) {
          uint32_t cnt = 0;
          insertTagUpdateCount(tags, tag_value.value.get_value_vector().size(), value, cnt);

          /*
          auto uindex = gsl::narrow<uint64_t>(index);

          if (tag_bit == VCDBit::VCD_1) {
            if (!tags.contains(static_cast<unsigned int>(pow(
                    2, (static_cast<double>(tag_value.value.get_value_vector().size() - uindex - 1)))))) {
              tags.insert(static_cast<unsigned int>(pow(
                  2, (static_cast<double>(tag_value.value.get_value_vector().size() - uindex - 1)))));
            }
          }*/
        }
      }
    }
    tags_ci_inputs.emplace_back(tags);
  }

  for (const auto& [vcd_index, vcd_data]: std::views::enumerate(m_vcd_data)) {
    auto vcd_uindex = gsl::narrow<uint64_t>(vcd_index);
    tags.clear();
    result.clear();
    // some edges do not have simulation values assigned to them
    if (target_signal_vcd_ptr != nullptr) {
      result.emplace_back(&(vcd_data->get_signal_values(target_signal_vcd_ptr->hash)));
    }
    for (const auto* tag_vector: result) {
      for (const auto& tag_value: *tag_vector) {
        auto value_vector = tag_value.value.get_value_vector();
        //for (const auto& [index, tag_bit]: std::views::enumerate(value_vector))
        for (const auto& value: std::views::enumerate(value_vector)) {
          auto tagPattern = getTagPattern(tag_value.value.get_value_vector().size(), value);
          if (tagPattern != 0) {
            if (tags_ci_inputs[vcd_uindex].contains(tagPattern)) {
              insertTagUpdateCount(tags, tag_value.value.get_value_vector().size(), value, tag_cnt);
            }
          }
          /*
            if (tag_bit == VCDBit::VCD_1) {
              auto uindex = gsl::narrow<uint64_t>(index);
              if (!tags.contains(static_cast<unsigned int>(pow(
                      2, (static_cast<double>(tag_value.value.get_value_vector().size() - uindex - 1))))) &&
                  tags_ci_inputs[vcd_uindex].contains(static_cast<unsigned int>(pow(
                      2, (static_cast<double>(tag_value.value.get_value_vector().size() - uindex - 1)))))) {
                tags.insert(static_cast<unsigned int>(pow(
                    2, (static_cast<double>(tag_value.value.get_value_vector().size() - uindex - 1)))));
                tag_cnt++;
              }
            }*/
        }
      }
    }
  }
  return tag_cnt;
}

[[nodiscard]] SignalInfo VCDSignalsDataManager::get_signal(const SignalIdentifier& signal) const {
  SignalInfo signal_info;
  signal_info.reference = signal.name;
  signal_info.size = gsl::narrow<uint64_t>(get_vcd_signal_ptr(signal)->size);
  signal_info.hash = get_vcd_signal_ptr(signal)->hash;
  return signal_info;
}

[[nodiscard]] SignalValues SMTSignalsDataManager::get_signal_values(const SignalIdentifier& signal_id) const {
  SignalValues values;
  for (SignalTime time = 0; time < m_smt_timesteps; time++) {
    // Create the signal name
    std::string signal_name = ducode::create_smt_signal_name(
        1, signal_id.hierarchy,
        legalize_identifier(signal_id.name),
        time);
    // Use .find() instead of operator[] for const maps
    auto signal_it = m_smt_signal_name_to_expr_id.find(signal_name);
    if (signal_it == m_smt_signal_name_to_expr_id.end()) {
      throw std::runtime_error(fmt::format("SMT model does not have signal {}", signal_name));
    }
    // Get the edge ID from the map
    uint32_t edge_id = signal_it->second;
    // Get the corresponding expression from the SMT model
    z3::expr expr = (*m_signal_expr_vector)[edge_id];
    z3::model model = m_solver->get_model();
    // Evaluate in the model
    auto value = model.eval(expr).as_uint64();
    SignalBitVector valVector;
    // Convert each bit of the binary representation to a SignalBit
    const uint32_t bit_size = model.eval(expr).get_sort().bv_size();
    valVector.resize(bit_size, SignalBit::BIT_0);
    for (uint32_t i = 0; i < bit_size; i++) {
      valVector[i] = ((value & (1ULL << i)) != 0u) ? SignalBit::BIT_1 : SignalBit::BIT_0;
    }

    // Convert to SignalValue
    SignalValue signal_value = SignalValue(valVector);
    // Add to the values vector
    values.m_timed_signal_values.emplace_back(TimedSignalValue(time, signal_value));
  }

  if (values.m_timed_signal_values.empty()) {
    throw std::runtime_error(fmt::format("SMT model does not have signal {}", signal_id.to_string()));
  }
  // Return the values vector
  return values;
}

[[nodiscard]] SignalInfo SMTSignalsDataManager::get_signal(const SignalIdentifier& signal) const {
  SignalInfo signal_info;
  std::string smt_signal_name = ducode::create_smt_signal_name(1, signal.hierarchy, signal.name, 0);
  if (!m_smt_signal_name_to_expr_id.contains(smt_signal_name)) {
    throw std::out_of_range(fmt::format("Signal name does not exist: {} in SMTSignalsDataManager.get_signal()", smt_signal_name));
  }
  uint32_t signal_smt_id = m_smt_signal_name_to_expr_id.at(smt_signal_name);
  signal_info.reference = signal.name;
  signal_info.size = (*m_signal_expr_vector)[signal_smt_id].get_sort().bv_size();
  // signal_info.hash = (*m_signal_expr_vector)[signal_smt_id].to_string();
  signal_info.hash = fmt::format("{}_{}", boost::algorithm::join(signal.hierarchy, "_"), signal.name);

  return signal_info;
}

[[nodiscard]] const std::vector<SignalTime>& SMTSignalsDataManager::get_timestamps() const {
  //Why was this static!? Calling this twice means to double time stamps ... static std::vector<SignalTime> timestamps;
  // Move the whole initialization to the constructor.

  return m_timestamps;
}

[[nodiscard]] SignalValue SMTSignalsDataManager::get_signal_value(std::string_view signal_name, const std::vector<std::string>& hierarchy, SignalTime timestep) const {
  std::string signal = create_smt_signal_name(1, hierarchy, signal_name, timestep);

  auto signal_it = m_smt_signal_name_to_expr_id.find(signal);
  if (signal_it == m_smt_signal_name_to_expr_id.end()) {
    throw std::runtime_error("Signal not found in SMT model");
  }

  // Get the edge ID from the map
  uint32_t edge_id = signal_it->second;
  // Get the corresponding expression from the SMT model
  z3::expr expr = (*m_signal_expr_vector)[edge_id];
  // Evaluate in the model
  std::string bitstring = (m_solver->get_model()).eval(expr).to_string();
  SignalValue signal_value(bitstring);
  return signal_value;
}

[[nodiscard]] SignalValue SMTSignalsDataManager::get_signal_value(const SignalIdentifier& signal_id, SignalTime timestep) const {
  return SMTSignalsDataManager::get_signal_value(signal_id.name, signal_id.hierarchy, timestep);
}

/* GF to be removed
[[nodiscard]] SignalValue SMTSignalsDataManager::get_signal_value_as_vector(const SignalIdentifier& signal_id, SignalTime timestep) const {
  std::string signal = create_smt_signal_name(1, signal_id.hierarchy, signal_id.name, timestep);

  auto signal_it = m_smt_signal_name_to_expr_id.find(signal);
  if (signal_it == m_smt_signal_name_to_expr_id.end()) {
    throw std::runtime_error("Signal not found in SMT model");
  }

  // Get the edge ID from the map
  uint32_t edge_id = signal_it->second;
  // Get the corresponding expression from the SMT model
  z3::expr expr = (*m_signal_expr_vector)[edge_id];
  // Evaluate in the model
  auto value = m_solver->get_model().eval(expr).as_uint64();
  SignalBitVector valVector;
  // Convert each bit of the binary representation to a SignalBit
  const uint32_t bit_size = m_solver->get_model().eval(expr).get_sort().bv_size();
  valVector.resize(bit_size, SignalBit::BIT_0);
  for (uint32_t i = 0; i < bit_size; i++) {
    valVector[i] = ((value & (1ULL << i)) != 0u) ? SignalBit::BIT_1 : SignalBit::BIT_0;
  }

  // Convert to SignalValue
  SignalValue signal_value = SignalValue(valVector);
  return signal_value;
}*/

// checks if the tag injected at the input signal at input_time is present at the target signal at target_time

[[nodiscard]] std::optional<bool> VCDSignalsDataManager::is_tag_propagated(SignalIdentifier& input_signal, SignalTime input_time, SignalIdentifier& target_signal, SignalTime target_time) {
  if (m_vcd_data.empty()) {
    throw std::runtime_error("No VCD data found");
  }

  auto get_scope_hierarchy = [](const VCDScope& scope) -> std::vector<std::string> {
    std::deque<std::string> result;
    const VCDScope* temp = &scope;
    while (temp != nullptr) {
      result.emplace_front(temp->name);
      temp = temp->parent;
    }
    return {result.begin() + 2, result.end()};
  };

  auto compare_hierarchies = [](const auto& hierarchy1, const auto& hierarchy2) {
    auto drop1 = hierarchy1 | std::views::drop(1);
    auto drop2 = hierarchy2 | std::views::drop(1);

    return std::ranges::equal(drop1, drop2);
  };

  uint32_t vcd_data_index = 0;
  // looping over the vector of VCD files to find the VCD file in which the input signal has a tag at the given time (should only be one vcd file in the vector)
  for (const auto& vcd_data: m_vcd_data) {
    std::deque<VCDSignal> signals = vcd_data->get_signals();

    // find signal with signal.reference == input_signal.name && scope hierarchy == input_signal.hierarchy
    const auto source_signal = std::ranges::find_if(signals, [&](const auto& signal) { return (signal.reference == legalize_identifier(input_signal.name) + ift_tag) && (compare_hierarchies(get_scope_hierarchy(*signal.scope), input_signal.hierarchy)); });

    // get the tag data for the input signal
    auto input_tag_values = m_vcd_data[vcd_data_index]->get_signal_values(source_signal->hash);
    // check if the input signal has a tag at the given time; tags are only present for one time step so if there is a tag the given time has to be in the vcd file explicitly
    auto input_tag_it = std::ranges::find_if(input_tag_values, [&](const VCDTimedValue& tag) { return tag.time == input_time; });
    // if the tag is not present in the VCD file, continue to the next VCD file
    if (input_tag_it == input_tag_values.end()) {
      vcd_data_index++;
      continue;
    }
    // if input_tag_it is not at .end() check if the value has exactly one tag as a 1 bit
    if (input_tag_it->value.get_type() == VCDValueType::VECTOR) {
      auto input_tag_vector = input_tag_it->value.get_value_vector();
      if (std::ranges::count_if(input_tag_vector, [](auto bit) { return bit == VCDBit::VCD_1; }) != 1) {
        throw std::runtime_error(fmt::format("The input signal {} has more than one tag at the given time. An INPUT signal should always only have one tag.", input_signal.name));// change this error throw to an return unknown using optional<bool>
      }
      if (std::ranges::count_if(input_tag_vector, [](auto bit) { return bit == VCDBit::VCD_1; }) == 0 && (input_time != 0)) {
        throw std::runtime_error(fmt::format("The input signal {} has no tag at the given time. An INPUT signal should always have a tag.", input_signal.name));
      }
    }
    break;
  }

  // find the input signal in the VCD data
  // find signal with signal.reference == input_signal.name && scope hierarchy == input_signal.hierarchy
  const auto source_signal_it = std::ranges::find_if(m_vcd_data[vcd_data_index]->get_signals(), [&](const auto& signal) { return (signal.reference == legalize_identifier(input_signal.name) + ift_tag) && (compare_hierarchies(get_scope_hierarchy(*signal.scope), input_signal.hierarchy)); });

  // find the target signal in the VCD data
  const auto target_signal_it = std::ranges::find_if(m_vcd_data[vcd_data_index]->get_signals(), [&](const auto& signal) { return (signal.reference == legalize_identifier(target_signal.name) + ift_tag) && (compare_hierarchies(get_scope_hierarchy(*signal.scope), target_signal.hierarchy)); });

  if (source_signal_it == m_vcd_data[vcd_data_index]->get_signals().end()) {
    throw std::runtime_error(fmt::format("The referenced input signal {} was not found in the VCD data", std::string(input_signal.name)));
  }
  if (target_signal_it == m_vcd_data[vcd_data_index]->get_signals().end()) {
    throw std::runtime_error(fmt::format("The referenced target signal {} was not found in the VCD data", target_signal.name));
  }

  // get the tag data for the input signal
  auto input_tag_values = m_vcd_data[vcd_data_index]->get_signal_values(source_signal_it->hash);

  // get the tag data for the target signal
  auto target_tag_values = m_vcd_data[vcd_data_index]->get_signal_values(target_signal_it->hash);

  // get input_tag
  auto input_tag_it = std::ranges::find_if(input_tag_values, [&](const VCDTimedValue& tag) { return tag.time == input_time; });
  // check if the target signal contains the tag of the input signal at the given target_time
  auto target_tag_it = std::ranges::find_if(target_tag_values, [&](const VCDTimedValue& tag) { return tag.time == target_time; });

  // compare the tag values of the input and target signals
  if (input_tag_it != input_tag_values.end() && target_tag_it != target_tag_values.end()) {
    spdlog::debug("Checking if the tag is propagated from {} to {}", input_signal.name, target_signal.name);
    auto input_tag_vector = input_tag_it->value.get_value_vector();
    auto target_tag_vector = target_tag_it->value.get_value_vector();

    // If sizes don't match, pad the smaller vector with zeros
    if (input_tag_vector.size() != target_tag_vector.size()) {
      // Determine which vector is smaller
      if (input_tag_vector.size() < target_tag_vector.size()) {
        // Pad input_tag_vector with zeros at the front
        VCDBitVector padded_input_vector(target_tag_vector.size(), VCDBit::VCD_0);
        std::ranges::copy(input_tag_vector,
                          padded_input_vector.end() - gsl::narrow<int64_t>(input_tag_vector.size()));
        input_tag_vector = padded_input_vector;
      } else {
        // Pad target_tag_vector with zeros at the front
        VCDBitVector padded_target_vector(input_tag_vector.size(), VCDBit::VCD_0);
        std::ranges::copy(target_tag_vector,
                          padded_target_vector.end() - gsl::narrow<int64_t>(target_tag_vector.size()));
        target_tag_vector = padded_target_vector;
      }
    }

    // Now compare the vectors (they're guaranteed to be the same size)
    for (auto i = 0ul; i < input_tag_vector.size(); i++) {
      if (input_tag_vector[i] == VCDBit::VCD_1 && target_tag_vector[i] == VCDBit::VCD_1) {
        return true;
      }
    }
  }

  return std::nullopt;//false;
}

[[nodiscard]] std::optional<bool> SMTSignalsDataManager::is_tag_propagated([[maybe_unused]] SignalIdentifier& input_signal, [[maybe_unused]] SignalTime input_time, [[maybe_unused]] SignalIdentifier& target_signal, [[maybe_unused]] SignalTime target_time) {
  [[maybe_unused]] std::string starting_signal_smt_name = ducode::create_smt_signal_name(
      1, input_signal.hierarchy,
      legalize_identifier(input_signal.name),
      gsl::narrow<uint32_t>(input_time));
  [[maybe_unused]] std::string target_signal_smt_name = ducode::create_smt_signal_name(
      1, target_signal.hierarchy,
      legalize_identifier(target_signal.name),
      gsl::narrow<uint32_t>(target_time));

  // create input constraints of the form (input1 != input2)
  z3::expr_vector input_constraints(m_solver->ctx());
  std::unordered_map<std::string, uint32_t> input_to_constraint_id;
  for (uint32_t unroll_factor_cnt = 0; unroll_factor_cnt < m_smt_timesteps; unroll_factor_cnt++) {
    for (uint32_t i = 0; i < m_input_signals.size(); i++) {
      z3::expr input_constraint_design_instance_1 = (*m_signal_expr_vector)[m_smt_signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, m_top_module_name), m_input_signals[i].name, unroll_factor_cnt)]];
      z3::expr input_constraint_design_instance_2 = (*m_signal_expr_vector)[m_smt_signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, m_top_module_name), m_input_signals[i].name, unroll_factor_cnt)]];
      input_constraints.push_back(input_constraint_design_instance_1 != input_constraint_design_instance_2);
      input_to_constraint_id[m_input_signals[i].name] = i;
    }
  }
  // create output constraints of the form (output1 != output2)
  std::unordered_map<std::string, uint32_t> output_to_constraint_id;
  z3::expr_vector output_constraints(m_solver->ctx());
  for (uint32_t unroll_factor_cnt = 0; unroll_factor_cnt < m_smt_timesteps; unroll_factor_cnt++) {
    for (uint32_t i = 0; i < m_output_signals.size(); i++) {
      z3::expr output_constraint_design_instance_1 = (*m_signal_expr_vector)[m_smt_signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, m_top_module_name), m_output_signals[i].name, unroll_factor_cnt)]];
      z3::expr output_constraint_design_instance_2 = (*m_signal_expr_vector)[m_smt_signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, m_top_module_name), m_output_signals[i].name, unroll_factor_cnt)]];
      output_constraints.push_back(output_constraint_design_instance_1 != output_constraint_design_instance_2);
      output_to_constraint_id[m_output_signals[i].name] = i;
    }
  }
  // needed for z3::pbeq; set of coefficients, one for each input
  std::vector<int> val;
  val.reserve(m_input_signals.size());
  for (uint32_t i = 0; i < m_input_signals.size(); i++) {
    val.emplace_back(1);
  }
  m_solver->push();
  //setting the constraint for the input given by the tag
  m_solver->add(input_constraints[input_to_constraint_id[starting_signal_smt_name]] == m_solver->ctx().bool_val(true));
  // setting the constraint for the current output
  m_solver->add(output_constraints[output_to_constraint_id[target_signal_smt_name]] == m_solver->ctx().bool_val(true));
  // using the input corresponding to the tag as the ExactlyOneOf constraint of the inputs
  m_solver->add(z3::pbeq(input_constraints, val.data(), 1));
  // as 2nd step adding the tag input constraint as true;
  if (m_solver->check() == z3::sat) {
    m_solver->pop();
    return true;
  }
  if (m_solver->check() == z3::unknown) {
    m_solver->pop();
    return std::nullopt;
  }
  m_solver->pop();

  return false;
}

void SMTSignalsDataManager::add_state_constraints() {
  // adds constraints to the solver for the intitial state of the design: initial state has to be the same in D1 and D2
}

// a second version with specific values for the state variables

}// namespace ducode
