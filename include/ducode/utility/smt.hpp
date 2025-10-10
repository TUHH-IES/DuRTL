#pragma once

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>

#include "ducode/utility/VCD_utility.hpp"
#include "ducode/utility/parser_utility.hpp"
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <spdlog/spdlog.h>
#include <z3++.h>

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

template<>
struct fmt::formatter<z3::expr_vector> : ostream_formatter {
};

template<>
struct fmt::formatter<z3::model> : ostream_formatter {
};

namespace ducode {
inline void write_to_smt2(const std::string &smt2_filename, z3::solver &solver) {
  // Write the SMT-LIB2 context to a file for debugging/inspection
  std::ofstream smt2_file(smt2_filename);
  if (smt2_file.is_open()) {
    smt2_file << solver.to_smt2();
    smt2_file.close();
    spdlog::info("Written SMT-LIB2 context to file: {}", smt2_filename);
  } else {
    spdlog::error("Failed to open file: {}", smt2_filename);
  }
}

inline void write_to_smt2(const std::string &smt2_filename, z3::model &model) {
  // Write the SMT-LIB2 context to a file for debugging/inspection
  std::ofstream smt2_file(smt2_filename);
  if (smt2_file.is_open()) {
    smt2_file << model.to_string();
    smt2_file.close();
    spdlog::info("Written SMT-LIB2 context to file: {}", smt2_filename);
  } else {
    spdlog::error("Failed to open file: {}", smt2_filename);
  }
}

[[nodiscard]] inline std::string create_smt_signal_name(uint32_t design_instance_id, const std::vector<std::string> &hierarchy, std::string_view name, SignalTime time) {
  return fmt::format("|D{}_{}_{}_time_{}|", design_instance_id, boost::algorithm::join(hierarchy, "_"), name, time);
};

// inverts the order of the vcd_value so the LSB is at index/position 0 of the vector; extends the vcd_value by the missing leading zeros (MSBs) omitted in the VCD file
[[nodiscard]] inline std::vector<SignalBit> extend_signal_value_vector(const SignalBitVector &signal_value, const uint64_t &vector_size) {
  std::vector<SignalBit> tmp_vec;
  tmp_vec.reserve(vector_size);
  for (uint64_t idx = 0; idx < signal_value.size(); idx++) {
    tmp_vec.emplace_back(signal_value.at((signal_value.size() - 1) - idx));
  }

  if (signal_value.front() == SignalBit::BIT_X) {
    for (uint64_t idx = 0; idx < static_cast<uint64_t>(vector_size) - signal_value.size(); idx++) {
      tmp_vec.emplace_back(SignalBit::BIT_X);
    }
  } else {
    for (uint64_t idx = 0; idx < static_cast<uint64_t>(vector_size) - signal_value.size(); idx++) {
      tmp_vec.emplace_back(SignalBit::BIT_0);
    }
  }
  return tmp_vec;
}

[[nodiscard]] inline boost::dynamic_bitset<> get_signal_value_as_bitset(const SignalValue &signal_value, const uint64_t &bitset_size) {
  boost::dynamic_bitset<> signal_value_bitset(bitset_size);
  if (signal_value.get_type() == SignalValueType::VECTOR) {
    std::vector<SignalBit> tmp_vec = extend_signal_value_vector(signal_value.get_value_vector(), bitset_size);
    for (uint32_t i = 0; i < tmp_vec.size(); i++) {
      if (tmp_vec[(tmp_vec.size() - 1) - i] == SignalBit::BIT_1) {
        signal_value_bitset.set((tmp_vec.size() - 1) - i);
      } else {
        signal_value_bitset.reset((tmp_vec.size() - 1) - i);
      }
    }
  } else if (signal_value.get_type() == SignalValueType::SCALAR) {
    if (signal_value.get_value_bit() == SignalBit::BIT_X) {
      return signal_value_bitset;
    }
    signal_value_bitset = boost::dynamic_bitset<>(bitset_size, signal_value.get_value_bit() == SignalBit::BIT_1 ? 1 : 0);
  } else if (signal_value.get_type() == SignalValueType::REAL) {
    signal_value_bitset = boost::dynamic_bitset<>(bitset_size, gsl::narrow<uint64_t>(signal_value.get_value_real()));
  }
  return signal_value_bitset;
}

[[nodiscard]] inline boost::dynamic_bitset<> get_signal_value_bitmask(const SignalValue &signal_value, const uint64_t &bitset_size) {
  boost::dynamic_bitset<> signal_value_bitset(bitset_size);
  if (signal_value.get_type() == SignalValueType::VECTOR) {
    std::vector<SignalBit> tmp_vec = extend_signal_value_vector(signal_value.get_value_vector(), bitset_size);
    for (uint32_t i = 0; i < tmp_vec.size(); i++) {
      if (tmp_vec[(tmp_vec.size() - 1) - i] == SignalBit::BIT_1 || tmp_vec[(tmp_vec.size() - 1) - i] == SignalBit::BIT_0) {
        signal_value_bitset.set((tmp_vec.size() - 1) - i);
      } else {
        signal_value_bitset.reset((tmp_vec.size() - 1) - i);
      }
    }
  } else if (signal_value.get_type() == SignalValueType::SCALAR) {
    if (signal_value.get_value_bit() == SignalBit::BIT_X || signal_value.get_value_bit() == SignalBit::BIT_Z) {
      return signal_value_bitset;
    }
    signal_value_bitset = boost::dynamic_bitset<>(bitset_size, 1);
  } else if (signal_value.get_type() == SignalValueType::REAL) {
    signal_value_bitset = boost::dynamic_bitset<>(bitset_size, gsl::narrow<uint64_t>(signal_value.get_value_real()));
  }
  return signal_value_bitset;
}

inline void add_smt_signal_value(const std::string &name, DesignInstance &instance, z3::solver &solver,
                                 z3::context &ctx, z3::expr_vector &ev_edges, SignalTime timestep,
                                 std::unordered_map<std::string, uint32_t> &signal_name_to_expr_id,
                                 uint32_t instance_id, SignalTime stepsize = 1) {
  spdlog::info("adding simulation values to SMT instance! signal: {}", name);
  for (const auto &edge: subrange(boost::edges(instance.m_graph))) {
    if (instance.m_graph[edge].net_ptr->get_name() == name) {
      const auto signal_values = instance.get_signal_values(edge);
      boost::dynamic_bitset<> signal_value_bitset;
      std::string edge_name = create_smt_signal_name(instance_id, instance.m_graph[edge].hierarchy, legalize_identifier(instance.m_graph[edge].net_ptr->get_name()), timestep);
      // get the value at the correct timestep or if timestep is not contained than the next smaller timestep
      for (uint32_t signal_idx = 0; signal_idx < (signal_values).m_timed_signal_values.size(); signal_idx++) {
        if (!signal_name_to_expr_id.contains(edge_name)) {
          throw std::runtime_error("no smt signal with that name " + edge_name + " found");
        }
        if ((signal_values).m_timed_signal_values[signal_idx].time == timestep * stepsize) {
          const uint64_t bv_size = ev_edges[signal_name_to_expr_id[edge_name]].get_sort().bv_size();
          signal_value_bitset = get_signal_value_as_bitset((signal_values).m_timed_signal_values[signal_idx].value, bv_size);
          // setting the signal value as a constraint for the corresponding smt variable
          if (!signal_value_bitset.empty()) {
            // signal_value_bitset is never 'empty' so even signals that get an x value are set (probably to zero)
            for (uint32_t idx = 0; idx < signal_value_bitset.size(); idx++) {
              solver.add(ev_edges[signal_name_to_expr_id[edge_name]].extract(idx, idx) == ctx.bv_val(
                                                                                              static_cast<int>(signal_value_bitset[idx]), 1));
            }
          }
          break;
        }
        if ((signal_values).m_timed_signal_values[signal_idx].time > timestep * stepsize) {
          const uint64_t bv_size = ev_edges[signal_name_to_expr_id[edge_name]].get_sort().bv_size();
          signal_value_bitset = get_signal_value_as_bitset((signal_values).m_timed_signal_values[signal_idx - 1].value, bv_size);
          // setting the signal value as a constraint for the corresponding smt variable
          if (!signal_value_bitset.empty()) {
            for (uint32_t idx = 0; idx < signal_value_bitset.size(); idx++) {
              solver.add(ev_edges[signal_name_to_expr_id[edge_name]].extract(idx, idx) == ctx.bv_val(
                                                                                              static_cast<int>(signal_value_bitset[idx]), 1));
            }
          }
          break;
        }
      }
      // if the for loop did not find a fitting timestep than the timestep is after the time of the last entry so take the last entry for the value
      auto signal_time = (signal_values).m_timed_signal_values.back().time;
      if (signal_time < timestep * stepsize) {
        const uint64_t bv_size = ev_edges[signal_name_to_expr_id[edge_name]].get_sort().bv_size();
        signal_value_bitset = get_signal_value_as_bitset((signal_values).m_timed_signal_values.back().value, bv_size);
        if (!signal_value_bitset.empty()) {
          for (uint32_t idx = 0; idx < signal_value_bitset.size(); idx++) {
            solver.add(ev_edges[signal_name_to_expr_id[edge_name]].extract(idx, idx) == ctx.bv_val(
                                                                                            static_cast<int>(signal_value_bitset[idx]), 1));
          }
        }
      }
      break;
    }
  }
}

inline void add_smt_signal_value(const SignalIdentifier &signal_id, SignalValue &signal_value, z3::solver &solver,
                                 z3::context &ctx, z3::expr_vector &ev_edges, SignalTime timestep,
                                 std::unordered_map<std::string, uint32_t> &signal_name_to_expr_id,
                                 uint32_t instance_id) {
  boost::dynamic_bitset<> signal_value_bitset;
  std::string edge_name = create_smt_signal_name(instance_id, signal_id.hierarchy, legalize_identifier(signal_id.name), timestep);
  // get the value at the correct timestep or if timestep is not contained than the next smaller timestep
  if (!signal_name_to_expr_id.contains(edge_name)) {
    throw std::runtime_error("no smt signal with that name " + edge_name + " found");
  }
  const uint64_t bv_size = ev_edges[signal_name_to_expr_id[edge_name]].get_sort().bv_size();
  signal_value_bitset = get_signal_value_as_bitset(signal_value, bv_size);
  spdlog::info("adding simulation values to SMT instance! signal: {}={}", signal_id.name, signal_value_bitset.to_ulong());
  boost::dynamic_bitset<> tmp_bitmask = get_signal_value_bitmask(signal_value, bv_size);
  // setting the signal value as a constraint for the corresponding smt variable
  if (!signal_value_bitset.empty()) {
    // signal_value_bitset is never 'empty' so even signals that get an x value are set (probably to zero) <= should not happen for x values those should stay symbolic
    for (uint32_t idx = 0; idx < signal_value_bitset.size(); idx++) {
      if (static_cast<int>(tmp_bitmask[idx]) == 1) {
        std::string expr_name = fmt::format("{}_Bit{} = {}", ev_edges[signal_name_to_expr_id[edge_name]].to_string(), idx, static_cast<int>(signal_value_bitset[idx]));
        z3::expr new_expr = ev_edges[signal_name_to_expr_id[edge_name]].extract(idx, idx) == ctx.bv_val(static_cast<int>(signal_value_bitset[idx]), 1);
        // spdlog::info("adding {}", expr_name);
        solver.add(new_expr, expr_name.c_str());
      }
    }
  } else {
    throw std::runtime_error("add_smt_signal_value: bitset is empty!");
  }
}
// compare every single bit from an z3 variable and its corresponding signal in the VCD data
[[nodiscard]] inline bool compare_bits(z3::solver &solver, z3::expr &z3_signal, const SignalValue &signal, uint64_t signal_size) {
  bool bit_matches = true;

  if (signal.get_type() == SignalValueType::SCALAR) {
    if (z3_signal.get_sort().bv_size() != 1) {
      throw std::runtime_error(
          "vcd value bitvector and z3 signal bitvector do not have the same length of 1! (SCALAR)");
    }
    if (signal.get_value_bit() == SignalBit::BIT_1 || signal.get_value_bit() == SignalBit::BIT_0) {
      auto tmp_bit_value = solver.get_model().eval(z3_signal).as_int64() == 1 ? SignalBit::BIT_1 : SignalBit::BIT_0;
      if (signal.get_value_bit() != tmp_bit_value) {
        bit_matches = false;
        spdlog::info("mismatched {} Bit (VCD::SCALAR) : (VCD) {} != {} (SMT)", z3_signal.to_string(), (signal.get_value_bit() == SignalBit::BIT_1 ? 1 : 0), (tmp_bit_value == SignalBit::BIT_1 ? 1 : 0));
      } else {
        spdlog::info("matched Bit (VCD::SCALAR) : (VCD) {} == {} (SMT)", (signal.get_value_bit() == SignalBit::BIT_1 ? 1 : 0), (tmp_bit_value == SignalBit::BIT_1 ? 1 : 0));
      }
    }
  } else if (signal.get_type() == SignalValueType::VECTOR) {
    std::vector<SignalBit> tmp_vec = extend_signal_value_vector(signal.get_value_vector(), signal_size);
    for (uint32_t bit_idx = 0; bit_idx < static_cast<uint32_t>(signal_size); bit_idx++) {
      auto tmp_bit_value = solver.get_model().eval(z3_signal.extract(bit_idx, bit_idx)).as_int64() == 1 ? SignalBit::BIT_1 : SignalBit::BIT_0;
      if (tmp_vec.at(bit_idx) == SignalBit::BIT_1 || tmp_vec.at(bit_idx) == SignalBit::BIT_0) {
        if (tmp_bit_value != tmp_vec.at(bit_idx)) {
          bit_matches = false;
          spdlog::info("Mismatch SMT: {} = {}", z3_signal.to_string(), solver.get_model().eval(z3_signal).to_string());
          spdlog::info("         VCD: {}", signal.to_string());
          spdlog::debug("{} == {}", ((tmp_vec.at(bit_idx) == SignalBit::BIT_1) ? 1 : 0), ((tmp_bit_value == SignalBit::BIT_1) ? 1 : 0));

          spdlog::info("mismatched {} Bit {} (VCD::VECTOR) : (VCD) {} != {} (SMT)", z3_signal.to_string(), bit_idx, (tmp_vec.at(bit_idx) == SignalBit::BIT_1 ? 1 : 0), (tmp_bit_value == SignalBit::BIT_1 ? 1 : 0));
        }
      }
    }
    // if (bit_matches) {
    //   spdlog::debug("matched Bitvector (VCD) {} == {} (SMT)", boost::algorithm::join((tmp_vec), ""), solver.get_model().eval(z3_signal).to_string());
    // }
  }
  return bit_matches;
}

[[nodiscard]] inline bool compare_smt_signal_value(const std::string &name, DesignInstance &instance,
                                                   z3::solver &solver, z3::expr_vector &ev_edges, SignalTime timestep,
                                                   std::unordered_map<std::string, uint32_t> &
                                                       signal_name_to_expr_id,
                                                   SignalTime stepsize) {
  bool compare_result = true;
  for (const auto &edge: subrange(boost::edges(instance.m_graph))) {
    if (instance.m_graph[edge].net_ptr->get_name() == name) {
      const auto signal_values = instance.get_signal_values(edge);
      bool timestep_found = false;
      std::string edge_name = create_smt_signal_name(1, instance.m_graph[edge].hierarchy, legalize_identifier(instance.m_graph[edge].net_ptr->get_name()), timestep);

      // loop that either finds the value at the correct timestep or if timestep is not contained than the next smaller timestep
      for (uint32_t signal_idx = 0; signal_idx < (signal_values).m_timed_signal_values.size(); signal_idx++) {
        if (!signal_name_to_expr_id.contains(edge_name)) {
          throw std::runtime_error("no smt signal with that name " + edge_name + " found");
        }
        // timestep size has to be read out from the vcd data of the clock signal (replace 2 with the correct timestep size)
        if ((signal_values).m_timed_signal_values[signal_idx].time == timestep * stepsize) {
          timestep_found = true;
          // setting the signal value as a constraint for the corresponding smt variable
          z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
          compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values[signal_idx].value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
          if (!compare_result) {
            spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                         ev_edges[signal_name_to_expr_id[edge_name]].to_string());
          }
          break;
        }
        if ((signal_values).m_timed_signal_values[signal_idx].time > timestep * stepsize) {
          timestep_found = true;
          // setting the signal value as a constraint for the corresponding smt variable
          z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
          compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values[signal_idx - 1].value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
          if (!compare_result) {
            spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                         ev_edges[signal_name_to_expr_id[edge_name]].to_string());
          }
          break;
        }
      }
      // if the for loop did not find a fitting timestep than the timestep is after the time of the last entry so take the last entry for the value
      if (!timestep_found) {
        z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
        compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values.back().value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
        if (!compare_result) {
          spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                       ev_edges[signal_name_to_expr_id[edge_name]].to_string());
        }
      }
      break;
    }
  }
  return compare_result;
}

[[nodiscard]] inline bool compare_smt_signal_value(const SignalIdentifier &signal_id, DesignInstance &instance, z3::solver &solver, z3::expr_vector &ev_edges, SignalTime timestep, SignalTime stepsize, SignalTime unroll_step, std::unordered_map<std::string, uint32_t> &signal_name_to_expr_id) {
  bool compare_result = true;
  const auto signal_value = instance.get_signal_value(signal_id, timestep + (unroll_step * stepsize));
  if ((timestep + (unroll_step * stepsize)) > instance.m_signal_tag_map->get_timestamps().back()) {
    return true;
  }
  // instance.get_design().get_top_module().get_nets().back().is_hidden()
  std::string edge_name = create_smt_signal_name(1, signal_id.hierarchy, legalize_identifier(signal_id.name), unroll_step + 1);

  // loop that either finds the value at the correct timestep or if timestep is not contained than the next smaller timestep
  if (!signal_name_to_expr_id.contains(edge_name)) {
    throw std::runtime_error("no smt signal with that name " + edge_name + " found");
  }
  // setting the signal value as a constraint for the corresponding smt variable
  z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
  compare_result = compare_bits(solver, tmp_expr, signal_value, instance.m_signal_tag_map->get_signal_size({.name = signal_id.name, .hierarchy = signal_id.hierarchy}));
  if (!compare_result) {
    spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                 ev_edges[signal_name_to_expr_id[edge_name]].to_string());
  }

  // if the for loop did not find a fitting timestep than the timestep is after the time of the last entry so take the last entry for the value
  compare_result = compare_bits(solver, tmp_expr, signal_value, instance.m_signal_tag_map->get_signal_size({.name = signal_id.name, .hierarchy = signal_id.hierarchy}));
  if (!compare_result) {
    spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                 ev_edges[signal_name_to_expr_id[edge_name]].to_string());
  }
  return compare_result;
}

//this function is only for the smt_to_simulation validation direction
[[nodiscard]] inline bool compare_smt_signal_value(const SignalIdentifier &signal_id, DesignInstance &instance, std::shared_ptr<z3::solver> &solver, std::shared_ptr<z3::expr_vector> &ev_edges, SignalTime timestep, SignalTime unroll_step, std::unordered_map<std::string, uint32_t> &signal_name_to_expr_id) {
  bool compare_result = true;
  const auto signal_value = instance.get_signal_value(signal_id, timestep);

  // instance.get_design().get_top_module().get_nets().back().is_hidden()
  std::string edge_name = create_smt_signal_name(1, signal_id.hierarchy, legalize_identifier(signal_id.name), unroll_step);// + 1); Why plus 1!?

  // loop that either finds the value at the correct timestep or if timestep is not contained than the next smaller timestep
  if (!signal_name_to_expr_id.contains(edge_name)) {
    throw std::runtime_error("no smt signal with that name " + edge_name + " found");
  }
  // setting the signal value as a constraint for the corresponding smt variable
  z3::expr tmp_expr = (*ev_edges)[signal_name_to_expr_id[edge_name]];
  compare_result = compare_bits(*solver, tmp_expr, signal_value, instance.m_signal_tag_map->get_signal_size({.name = signal_id.name, .hierarchy = signal_id.hierarchy}));
  if (!compare_result) {
    spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                 (*ev_edges)[signal_name_to_expr_id[edge_name]].to_string());
  }
  return compare_result;
}

[[nodiscard]] inline bool compare_smt_signal_value_comb(const SignalIdentifier &signal_id, DesignInstance &instance,
                                                        z3::solver &solver, z3::expr_vector &ev_edges, SignalTime timestep, SignalTime stepsize, SignalTime unroll_step,
                                                        std::unordered_map<std::string, uint32_t> &
                                                            signal_name_to_expr_id) {
  bool compare_result = true;
  const auto signal_value = instance.get_signal_value(signal_id, timestep + (unroll_step * stepsize));
  if ((timestep + (unroll_step * stepsize)) > instance.m_signal_tag_map->get_timestamps().back()) {
    return true;
  }
  // instance.get_design().get_top_module().get_nets().back().is_hidden()
  std::string edge_name = create_smt_signal_name(1, signal_id.hierarchy, legalize_identifier(signal_id.name), unroll_step);

  // loop that either finds the value at the correct timestep or if timestep is not contained than the next smaller timestep
  if (!signal_name_to_expr_id.contains(edge_name)) {
    throw std::runtime_error("no smt signal with that name " + edge_name + " found");
  }
  // setting the signal value as a constraint for the corresponding smt variable
  z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
  compare_result = compare_bits(solver, tmp_expr, signal_value, instance.m_signal_tag_map->get_signal_size({.name = signal_id.name, .hierarchy = signal_id.hierarchy}));
  if (!compare_result) {
    spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                 ev_edges[signal_name_to_expr_id[edge_name]].to_string());
  }

  // if the for loop did not find a fitting timestep than the timestep is after the time of the last entry so take the last entry for the value
  compare_result = compare_bits(solver, tmp_expr, signal_value, instance.m_signal_tag_map->get_signal_size({.name = signal_id.name, .hierarchy = signal_id.hierarchy}));
  if (!compare_result) {
    spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                 ev_edges[signal_name_to_expr_id[edge_name]].to_string());
  }
  return compare_result;
}

[[nodiscard]] inline std::vector<std::string> get_all_flipflop_names(DesignInstance &instance) {
  std::vector<std::string> result_vector;

  for (const auto &vertex: subrange(boost::vertices(instance.m_graph))) {
    if (instance.m_graph[vertex].cell_ptr != nullptr) {
      if (instance.m_graph[vertex].cell_ptr->has_register()) {
        if (out_degree(vertex, instance.m_graph) >= 1) {
          auto edge = boost::out_edges(vertex, instance.m_graph);
          const auto &name = instance.m_graph[*edge.first].net_ptr->get_name();
          result_vector.emplace_back(name);
        } else {
          throw std::runtime_error("Unexpected number of output edges.");
        }
      }
    }
  }
  return result_vector;
}

// validate the smt design instance by using the input and output values of a simulation trace as constraints on the smt instance; if the smt instance is SAT then the design should be correct in the instance
// update the validation only uses the input values as constraints and then compare the smt model with the output values from the simulation
inline bool smt_sim_validation(Design &design, DesignInstance &instance, z3::expr_vector &ev_edges,
                               z3::solver &solver, z3::context &ctx, const std::vector<SignalIdentifier> &input_ids,
                               std::unordered_map<std::string, uint32_t> &signal_name_to_expr_id, SignalTime unroll_factor, SignalTime stepsize = 1) {
  auto top_module_outputs = design.get_module(design.get_top_module_name()).get_output_names();
  auto clk_signal = instance.get_design().get_clk_signal_name();
  bool compare_result = true;
  std::vector<SignalIdentifier> ff_ids = instance.get_all_flipflop_ids();

  std::unordered_map<SignalIdentifier, SignalValue> input_value_map;
  SignalTime clockbegin = 0;
  if (!clk_signal.empty()) {
    clockbegin = instance.get_signal_value({.name = clk_signal, .hierarchy = {design.get_top_module_name()}}, 0) == SignalValue(SignalBit::BIT_1) ? 0 : 1;
  }
  // get number of vcd timeunits per clockcycle
  // timesteps should be clockcycles not straight up timeunits in the vcd data (clk cycle a minimum of 2 timeunits)
  for (SignalTime timestep = (clockbegin * (stepsize / 2)); timestep < instance.m_signal_tag_map->get_timestamps().back(); timestep += (unroll_factor * stepsize)) {
    solver.push();
    auto value_change = [&]() -> bool {
      bool input_value_changed = false;
      for (const auto &input_id: input_ids) {
        if (input_value_map.empty()) {
          return true;
        }
        if (!input_value_map.contains(input_id)) {
          continue;
        }
        ducode::SignalValue val = instance.get_signal_value(input_id, timestep);
        if (val.get_type() == SignalValueType::SCALAR && val.get_value_bit() == SignalBit::BIT_X) {
          continue;// skip x values
        }
        auto x_it = false;
        if (val.get_type() == SignalValueType::VECTOR) {
          for (auto bit_value: val.get_value_vector()) {
            if (bit_value == SignalBit::BIT_X) {
              x_it = true;
              break;
            }
          }
        }
        if (val.get_type() == SignalValueType::VECTOR && x_it) {
          continue;// skip x values
        }
        if (input_value_map[input_id] != val) {
          input_value_changed = true;
          return input_value_changed;
        }
      }
      for (const auto &ff_id: ff_ids) {
        ducode::SignalValue val = instance.get_signal_value(ff_id, timestep);
        if (val.get_type() == SignalValueType::SCALAR && val.get_value_bit() == SignalBit::BIT_X) {
          continue;// skip x values
        }
        if (val.get_type() == SignalValueType::VECTOR && std::find(val.get_value_vector().begin(), val.get_value_vector().end(), SignalBit::BIT_X) != val.get_value_vector().end()) {
          continue;// skip x values
        }
        if (input_value_map[ff_id] != val) {
          input_value_changed = true;
          return input_value_changed;
        }
      }
      return input_value_changed;
    };

    if (!value_change() && clk_signal.empty()) {
      continue;
    }
    spdlog::info("current timestep: {}", timestep);

    auto loop_cond = unroll_factor == 1 ? 1 : unroll_factor - 1;
    for (SignalTime unroll_step = 0; unroll_step < (loop_cond); unroll_step++) {
      // instead of adding output values as constraints -> only add input_values and let SMT create a Model -> compare the SMT model values with simulation output values
      if (timestep + (unroll_step * stepsize) > instance.m_signal_tag_map->get_timestamps().back()) {
        return compare_result;
      }
      spdlog::info("set input values:");
      for (const auto &input_id: input_ids) {
        SignalValue input_value = instance.get_signal_value(input_id, timestep + (unroll_step * stepsize));
        input_value_map[input_id] = input_value;
        add_smt_signal_value(input_id, input_value, solver, ctx, ev_edges, unroll_step, signal_name_to_expr_id, 1);
      }

      spdlog::info("set flipflop (state) values:");
      for (const auto &ff_id: ff_ids) {
        SignalValue ff_value = instance.get_signal_value(ff_id, timestep + (unroll_step * stepsize));
        input_value_map[ff_id] = ff_value;

        add_smt_signal_value(ff_id, ff_value, solver, ctx, ev_edges, unroll_step + 1, signal_name_to_expr_id, 1);
        if (unroll_step == 0) {
          if (gsl::narrow<int32_t>(timestep) - gsl::narrow<int32_t>(stepsize) >= 0) {
            ff_value = instance.get_signal_value(ff_id, timestep - (stepsize));
          } else {
            ff_value = instance.get_signal_value(ff_id, 0);//if there is no previous rising edge take the value at simulation time 0
          }
          add_smt_signal_value(ff_id, ff_value, solver, ctx, ev_edges, unroll_step, signal_name_to_expr_id, 1);
        }
      }
    }
    // spdlog::debug("{}", solver.to_smt2());
    //ducode::DesignInstance::write_smt2(solver, "spi_smt2_latchtest.smt2");
    if (solver.check() == z3::sat) {
      spdlog::warn("Checking all net values sim - smt:");
      for (const auto &net: instance.m_design.get_module(instance.m_design.get_top_module_name()).get_output_names()) {
        if (net == instance.get_design().get_clk_signal_name()) {
          continue;
        }

        if (unroll_factor == 1) {
          spdlog::warn("{}", net);
          compare_result = compare_smt_signal_value_comb({.name = net, .hierarchy = {instance.m_design.get_top_module_name()}}, instance, solver, ev_edges, timestep, stepsize, 0, signal_name_to_expr_id);
        } else {
          for (SignalTime unroll_step = 0; unroll_step < unroll_factor - 1; unroll_step++) {
            spdlog::info("unroll_step={}; net={}", unroll_step, net);
            compare_result = compare_smt_signal_value({.name = net, .hierarchy = {instance.m_design.get_top_module_name()}}, instance, solver, ev_edges, timestep, stepsize, unroll_step, signal_name_to_expr_id);
          }
        }
        if (!compare_result) {
          spdlog::debug("{}", solver.to_smt2());
          return compare_result;
        }
      }
      solver.pop();
    } else {
      spdlog::info("Unsatisfiable core: {}", solver.unsat_core());
      throw std::runtime_error("SMT Instance is not SAT");
    }
  }
  return compare_result;
}

inline void validation_breadth_first_search_fan_in(DesignInstance &instance, z3::expr_vector &ev_edges, z3::solver &solver, const std::vector<std::string> &input_names, std::unordered_map<std::string, uint32_t> &signal_name_to_expr_id, SignalTime timestep, const ducode::instantiation_graph::vertex_descriptor &current_vertex, std::unordered_set<ducode::instantiation_graph::vertex_descriptor> &vertices_visited, std::unordered_set<const Net *> &visited_nets) {
  bool compare_result = true;
  std::vector<std::string> ff_names = get_all_flipflop_names(instance);

  auto vertex_filter = [&](const auto &vertex) {
    if (vertex.m_property.port_ptr != nullptr) {
      return ((vertex.m_property.port_ptr->m_direction == ducode::Port::Direction::input) && (std::find(input_names.begin(), input_names.end(), vertex.m_property.port_ptr->m_name) != input_names.end()) && (vertex.m_property.hierarchy.size() == 1));
    }
    return false;
  };

  std::deque<ducode::instantiation_graph::vertex_descriptor> next_vertices;
  next_vertices.emplace_back(current_vertex);

  for (const auto &[idx, vertex]: std::views::enumerate(instance.m_graph.m_vertices)) {
    if (vertex_filter(vertex)) {
      next_vertices.emplace_back(idx);
      vertices_visited.emplace(idx);
    }
  }
  if (solver.check() == z3::sat) {
    while (!next_vertices.empty()) {
      const auto &vertex_idx = next_vertices.front();
      next_vertices.pop_front();

      spdlog::warn("Checking net values for in_edges of vertex {} :", instance.m_graph[vertex_idx].name);

      for (const auto &edge: subrange(boost::in_edges(vertex_idx, instance.m_graph))) {
        if (!vertices_visited.contains(edge.m_target)) {
          vertices_visited.emplace(edge.m_target);
        }
        if (visited_nets.contains(instance.m_graph[edge].net_ptr)) {
          continue;
        }
        visited_nets.emplace(instance.m_graph[edge].net_ptr);

        const auto signal_values = instance.get_signal_values(edge);
        bool timestep_found = false;
        std::string edge_name = create_smt_signal_name(1, instance.m_graph[edge].hierarchy, legalize_identifier(instance.m_graph[edge].net_ptr->get_name()), timestep);

        // loop that either finds the value at the correct timestep or if timestep is not contained than the next smaller timestep
        for (uint32_t signal_idx = 0; signal_idx < (signal_values).m_timed_signal_values.size(); signal_idx++) {
          if (!signal_name_to_expr_id.contains(edge_name)) {
            throw std::runtime_error("no smt signal with that name " + edge_name + " found");
          }
          if ((signal_values).m_timed_signal_values[signal_idx].time == timestep * 2) {
            timestep_found = true;
            // setting the signal value as a constraint for the corresponding smt variable
            z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
            compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values[signal_idx].value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
            if (!compare_result) {
              spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                           ev_edges[signal_name_to_expr_id[edge_name]].to_string());
              if (!vertices_visited.contains(edge.m_target) && !compare_result) {
                next_vertices.emplace_back(edge.m_target);
              }
            }
            break;
          }
          if ((signal_values).m_timed_signal_values[signal_idx].time > timestep * 2) {
            timestep_found = true;
            // setting the signal value as a constraint for the corresponding smt variable
            z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
            compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values[signal_idx - 1].value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
            if (!compare_result) {
              spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                           ev_edges[signal_name_to_expr_id[edge_name]].to_string());
              if (!vertices_visited.contains(edge.m_target) && !compare_result) {
                next_vertices.emplace_back(edge.m_target);
              }
            }
            break;
          }
        }
        // if the for loop did not find a fitting timestep than the timestep is after the time of the last entry so take the last entry for the value
        if (!timestep_found) {
          z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
          compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values.back().value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
          if (!compare_result) {
            spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                         ev_edges[signal_name_to_expr_id[edge_name]].to_string());
            if (!vertices_visited.contains(edge.m_target) && !compare_result) {
              next_vertices.emplace_back(edge.m_target);
            }
          }
        }
      }
    }
  }
}

[[nodiscard]] inline bool validation_breadth_first_search(DesignInstance &instance, z3::expr_vector &ev_edges,
                                                          z3::solver &solver, z3::context &ctx,
                                                          const std::vector<std::string> &input_names,
                                                          std::unordered_map<std::string, uint32_t> &
                                                              signal_name_to_expr_id,
                                                          SignalTime unroll_factor) {
  bool compare_result = true;
  std::vector<std::string> ff_names = get_all_flipflop_names(instance);
  // get number of vcd timeunits per clockcycle
  // timesteps should be clockcycles not straight up timeunits in the vcd data (clk cycle a minimum of 2 timeunits)
  for (SignalTime timestep = 0; timestep < unroll_factor; timestep++) {
    solver.push();

    // instead of adding output values as constraints -> only add input_values and let SMT create a Model -> compare the SMT model values with simulation output values
    for (const auto &input_name: input_names) {
      add_smt_signal_value(input_name, instance, solver, ctx, ev_edges, timestep, signal_name_to_expr_id, 1);
      //timestep needs to be multiplied by clockcycle length
    }

    // for (auto &ff_name: ff_names) {
    //   add_smt_signal_value(ff_name, instance, solver, ctx, ev_edges, timestep, signal_name_to_expr_id, 1);
    //   //timestep needs to be multiplied by clockcycle length
    // }

    auto vertex_filter = [&](const auto &vertex) {
      if (vertex.m_property.port_ptr != nullptr) {
        return ((vertex.m_property.port_ptr->m_direction == ducode::Port::Direction::input) && (std::find(input_names.begin(), input_names.end(), vertex.m_property.port_ptr->m_name) != input_names.end()) && (vertex.m_property.hierarchy.size() == 1));
      }
      return false;
    };

    std::deque<ducode::instantiation_graph::vertex_descriptor> next_vertices;
    std::unordered_set<ducode::instantiation_graph::vertex_descriptor> vertices_visited;
    std::unordered_set<const Net *> visited_nets;

    for (const auto &[idx, vertex]: std::views::enumerate(instance.m_graph.m_vertices)) {
      if (vertex_filter(vertex)) {
        next_vertices.emplace_back(idx);
        vertices_visited.emplace(idx);
      }
    }
    if (solver.check() == z3::sat) {
      spdlog::warn("Checking all net values sim - smt starting at the inputs :");
      while (!next_vertices.empty()) {
        const auto &vertex_idx = next_vertices.front();
        next_vertices.pop_front();
        spdlog::warn("Checking net values for out_edges of vertex {} :", instance.m_graph.m_vertices[vertex_idx].m_property.name);
        for (const instantiation_graph::edge_descriptor &edge: subrange(boost::out_edges(vertex_idx, instance.m_graph))) {
          if (!vertices_visited.contains(edge.m_target)) {
            next_vertices.emplace_back(edge.m_target);
            vertices_visited.emplace(edge.m_target);
          }
          if (visited_nets.contains(instance.m_graph[edge].net_ptr)) {
            continue;
          }
          visited_nets.emplace(instance.m_graph[edge].net_ptr);


          const auto signal_values = instance.get_signal_values(edge);
          bool timestep_found = false;
          std::string edge_name = create_smt_signal_name(1, instance.m_graph[edge].hierarchy, legalize_identifier(instance.m_graph[edge].net_ptr->get_name()), timestep);

          // loop that either finds the value at the correct timestep or if timestep is not contained than the next smaller timestep
          for (uint32_t signal_idx = 0; signal_idx < (signal_values).m_timed_signal_values.size(); signal_idx++) {
            if (!signal_name_to_expr_id.contains(edge_name)) {
              throw std::runtime_error("no smt signal with that name " + edge_name + " found");
            }
            if ((signal_values).m_timed_signal_values[signal_idx].time == timestep * static_cast<SignalTime>(2)) {
              timestep_found = true;
              // setting the signal value as a constraint for the corresponding smt variable
              z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
              compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values[signal_idx].value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
              if (!compare_result) {
                spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                             ev_edges[signal_name_to_expr_id[edge_name]].to_string());
                validation_breadth_first_search_fan_in(instance, ev_edges, solver, input_names,
                                                       signal_name_to_expr_id, timestep, vertex_idx, vertices_visited,
                                                       visited_nets);
                spdlog::debug("{}", solver.to_smt2());
                throw std::runtime_error("Bits do not match! - Timestep found!");
              }
              break;
            }
            if ((signal_values).m_timed_signal_values[signal_idx].time > timestep * static_cast<SignalTime>(2)) {
              timestep_found = true;
              // setting the signal value as a constraint for the corresponding smt variable
              z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
              compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values[signal_idx - 1].value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
              if (!compare_result) {
                spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                             ev_edges[signal_name_to_expr_id[edge_name]].to_string());
                validation_breadth_first_search_fan_in(instance, ev_edges, solver, input_names,
                                                       signal_name_to_expr_id, timestep, vertex_idx, vertices_visited,
                                                       visited_nets);
                throw std::runtime_error("Bits do not match! - Timestep not found! Earlier timestep used");
              }
              break;
            }
          }
          // if the for loop did not find a fitting timestep than the timestep is after the time of the last entry so take the last entry for the value
          if (!timestep_found) {
            z3::expr tmp_expr = ev_edges[signal_name_to_expr_id[edge_name]];
            compare_result = compare_bits(solver, tmp_expr, (signal_values).m_timed_signal_values.back().value, instance.m_signal_tag_map->get_signal_size({.name = instance.m_graph[edge].net_ptr->get_name(), .hierarchy = instance.m_graph[edge].hierarchy}));
            if (!compare_result) {
              spdlog::warn("Signal {} and Z3 Variable {} do not match", edge_name,
                           ev_edges[signal_name_to_expr_id[edge_name]].to_string());
              validation_breadth_first_search_fan_in(instance, ev_edges, solver, input_names, signal_name_to_expr_id,
                                                     timestep, vertex_idx, vertices_visited, visited_nets);
              throw std::runtime_error("Bits do not match! - Timestep not found! Last timestep used");
            }
          }
        }
      }
      spdlog::debug("{}", solver.to_smt2());
    }
    solver.pop();
  }
  return compare_result;
}

inline bool smt_to_simulation_validation(DesignInstance &instance, std::shared_ptr<z3::expr_vector> &ev_edges, std::shared_ptr<z3::solver> &solver, std::unordered_map<std::string, uint32_t> &signal_name_to_expr_id, SignalTime unroll_factor) {
  // We assume that the solver returned a model simulated on the test bench.
  // Thus, the output values of the top module should be the same as in the test bench
  bool compare_result = true;
  if (solver->check() == z3::sat) {
    spdlog::warn("Checking all output values of the topmodule sim - smt:");
    for (const auto &net: instance.m_design.get_module(instance.m_design.get_top_module_name()).get_output_names()) {
      spdlog::warn("Checking net " + net);
      if (net == instance.get_design().get_clk_signal_name()) {
        continue;
      }

      for (SignalTime unroll_step = 0; unroll_step < unroll_factor; unroll_step++) {
        spdlog::info("unroll_step={}; net={}", unroll_step, net);
        compare_result = compare_smt_signal_value({.name = net, .hierarchy = {instance.m_design.get_top_module_name()}}, instance, solver, ev_edges, (unroll_step * 4) + 2, unroll_step, signal_name_to_expr_id);
      }

      if (!compare_result) {
        spdlog::debug("{}", solver->to_smt2());
        return compare_result;
      }
    }
  } else {
    spdlog::info("Unsatisfiable core: {}", solver->unsat_core());
    throw std::runtime_error("SMT Instance is not SAT");
  }

  return compare_result;
}


/** Ensure that the solver yields under fully fixed input and initial state only a single output value */
inline bool smt_to_simulation_unique_assignment(z3::context &ctx, z3::solver &solver, const std::vector<z3::expr> &stateInit_expr, const std::vector<std::vector<z3::expr>> &input_expr, const std::vector<std::vector<z3::expr>> &output_expr) {
  bool compare_result = true;

  assert(solver.check() == z3::sat);
  auto model = solver.get_model();

  //Incremental check
  solver.push();
  // Add constraint to solver to fix all state and input values
  for (const auto &vec_expr: input_expr) {
    for (const auto &expr: vec_expr) {
      z3::expr expr_val = model.eval(expr);
      z3::expr equality(expr == expr_val);
      solver.add(equality);
      spdlog::info("Adding constraint on inputs: " + equality.to_string());
    }
  }
  for (const auto &expr: stateInit_expr) {
    z3::expr expr_val = model.eval(expr);
    z3::expr equality(expr == expr_val);
    solver.add(equality);
    spdlog::info("Adding constraint on inputs: " + equality.to_string());
  }
  // Add constraint to force at least one difference in output values
  z3::expr_vector output_diff_constraints(ctx);
  for (const auto &vec_expr: output_expr) {
    for (const auto &expr: vec_expr) {
      z3::expr expr_val = model.eval(expr);
      z3::expr diff = (expr != expr_val);
      output_diff_constraints.push_back(diff);
      spdlog::info("Adding constraint on outputs: " + diff.to_string());
    }
  }
  assert(!output_diff_constraints.empty());
  solver.add(z3::mk_or(output_diff_constraints));

  // A deterministic design yields an unsat result now
  auto result = solver.check();

  if (result == z3::sat) {
    spdlog::info("Unexpectedly found a satisfying assingment");
    auto modelSat = solver.get_model();
    for (const auto &vec_expr: input_expr) {
      for (const auto &expr: vec_expr) {
        z3::expr expr_val = modelSat.eval(expr);
        spdlog::info("Input: " + expr.to_string() + " = " + expr_val.to_string());
      }
    }
    for (const auto &expr: stateInit_expr) {
      z3::expr expr_val = modelSat.eval(expr);
      spdlog::info("Initial state: " + expr.to_string() + " = " + expr_val.to_string());
    }
    // Add constraint to force at least one difference in output values
    for (const auto &vec_expr: output_expr) {
      for (const auto &expr: vec_expr) {
        z3::expr expr_val = modelSat.eval(expr);
        spdlog::info("Output: " + expr.to_string() + " = " + expr_val.to_string());
      }
    }
  }

  //Remove constraints from incremental check
  solver.pop();
  return (result == z3::unsat);
}

inline void smt_fixed_assignment(z3::context &ctx, z3::solver &solver, const z3::expr &expr, int64_t value) {
  // Modified from code generated using ChatGPT - 2025/09/10

  // Check that it's a bitvector
  assert(expr.is_bv());
  // Get bitwidth
  unsigned bitwidth = expr.get_sort().bv_size();

  // Create a matching constant bitvector
  z3::expr const_expr = ctx.bv_val(value, bitwidth);

  // Generate constraint
  z3::expr constraint = (expr == const_expr);

  spdlog::info(" Fixed assignment " + constraint.to_string());
  solver.add(constraint);
}

inline void smt_random_assignment(z3::context &ctx, z3::solver &solver, const z3::expr &expr) {
  // Modified from code generated using ChatGPT - 2025/09/10

  // Check that it's a bitvector
  assert(expr.is_bv());
  // Get bitwidth
  unsigned bitwidth = expr.get_sort().bv_size();

  // Setup RNG
  std::random_device random_dev;
  std::mt19937_64 gen(random_dev());
  const uint64_t const_one = 1ULL;
  const uint64_t const_zero = 0ULL;
  const unsigned const_maxbitwidth = 64;

  assert(bitwidth < const_maxbitwidth);

  uint64_t max_val = (bitwidth == const_maxbitwidth ? ~const_zero : ((const_one << bitwidth) - 1));
  std::uniform_int_distribution<uint64_t> dist(0, max_val);

  // Pick a random value within the bitwidth
  uint64_t const_val = dist(gen);

  // Create a matching constant bitvector
  z3::expr const_expr = ctx.bv_val(const_val, bitwidth);

  // Generate constraint expr == const_expr
  z3::expr constraint = (expr == const_expr);

  spdlog::info(" Random assignment " + constraint.to_string());
  solver.add(constraint);
}

}// namespace ducode
