#pragma once

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/utility/simulation.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/visitors.hpp>
#include <gsl/narrow>
#include <sys/types.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTimedValue.hpp>
#include <vcd-parser/VCDTypes.hpp>
#include <z3++.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#include <z3_api.h>

namespace ducode {

class SignalsDataManager {
public:
  SignalsDataManager() = default;
  virtual ~SignalsDataManager() = default;
  SignalsDataManager(const SignalsDataManager&) = default;
  SignalsDataManager& operator=(const SignalsDataManager&) = default;
  SignalsDataManager(SignalsDataManager&&) noexcept = default;
  SignalsDataManager& operator=(SignalsDataManager&&) noexcept = default;

  [[nodiscard]] virtual SignalInfo get_signal(const SignalIdentifier& /*signal*/) const = 0;
  [[nodiscard]] virtual SignalValues get_signal_values(const SignalIdentifier& signal_id) const = 0;
  [[nodiscard]] virtual std::vector<SignalValues> get_tag_values(const SignalIdentifier& /*signal_id*/) const { throw std::runtime_error("Should never be called"); };
  [[nodiscard]] virtual const std::vector<SignalTime>& get_timestamps() const = 0;

  [[nodiscard]] virtual SignalValue get_signal_value(const SignalIdentifier& /* signal_id */, SignalTime /* timestep */) const = 0;

  //GF remove /*[[nodiscard]] virtual SignalValue get_signal_value_as_vector(const SignalIdentifier& /*signal_id*/, SignalTime /*timestep*/) const {
  //  throw std::runtime_error("Should never be called");
  //};


  [[nodiscard]] virtual uint32_t get_tag_count_signal(const std::string& /*signal_name*/, const std::vector<std::string>& /*signal_hierarchy*/) const { throw std::runtime_error("Should never be called"); };
  [[nodiscard]] virtual uint32_t get_tag_count_signal_to_signal(const SignalIdentifier& /*source_signal*/, const SignalIdentifier& /*target_signal*/) const { throw std::runtime_error("Should never be called"); };

  [[nodiscard]] virtual TagList get_tag_sources(SignalIdentifier& /*signal_id*/) const { throw std::runtime_error("Shoudl never be called! : get_tag_sources()"); };

  [[nodiscard]] virtual bool is_empty() const { throw std::runtime_error("Should never be called"); };

  [[nodiscard]] virtual Stepsize get_stepsize(const std::string& /*clk_name*/) const { throw std::runtime_error("Should never be called!"); };

  [[nodiscard]] virtual uint64_t get_size() const = 0;

  [[nodiscard]] virtual std::vector<SignalInfo> get_signals() const = 0;

  [[nodiscard]] virtual TagSourceList get_information_flow_source_signals_for_target_signal_at_timestep(const std::vector<SignalIdentifier>& /*source_signals*/, const SignalIdentifier& /*target_signal*/, const SignalTime /*timestep*/) const { throw std::runtime_error("Should never be called"); };
  [[nodiscard]] virtual TagSourceList get_information_flow_source_signals_for_target_signal(const std::vector<SignalIdentifier>& /*source_signals*/, const SignalIdentifier& /*target_signal*/) const { throw std::runtime_error("Should never be called"); };

  [[nodiscard]] virtual bool bitwise_vcd_compare([[maybe_unused]] VCDBitVector& vec1, [[maybe_unused]] VCDBitVector& vec2) const { throw std::runtime_error("Should never be called"); };

  [[nodiscard]] virtual uint64_t get_signal_size(const SignalIdentifier& signal) const = 0;
  [[nodiscard]] virtual std::unordered_map<std::string, SignalValues> get_input_values(std::vector<SignalIdentifier> input_ids) const = 0;

  [[nodiscard]] virtual uint64_t get_vcd_data_size() const { throw std::runtime_error("Should never be called! ((SMT)SignalsDataManager::get_data_size())"); };

  [[nodiscard]] virtual std::optional<bool> is_tag_propagated(SignalIdentifier& input_signal, SignalTime input_time, SignalIdentifier& target_signal, SignalTime target_time) = 0;
};

class SMTSignalsDataManager : public SignalsDataManager {
private:
  std::shared_ptr<z3::context> m_context;
  std::shared_ptr<z3::solver> m_solver;
  std::unordered_map<std::string, uint32_t> m_smt_signal_name_to_expr_id;
  SignalTime m_smt_timesteps = 0;
  std::shared_ptr<z3::expr_vector> m_signal_expr_vector;
  std::vector<SignalIdentifier> m_input_signals;
  std::vector<SignalIdentifier> m_output_signals;
  std::string m_top_module_name;
  std::vector<SignalTime> m_timestamps;
  // perhaps remember last input_edge + input_time so that if the same is queried again the previous model can be reused

public:
  SMTSignalsDataManager() = default;
  explicit SMTSignalsDataManager(std::shared_ptr<z3::context> ctx, std::shared_ptr<z3::solver> solver, std::unordered_map<std::string, uint32_t> smt_signal_name_to_edge_id, uint32_t timesteps, std::shared_ptr<z3::expr_vector> ev_edges, std::vector<SignalIdentifier> input_names, std::vector<SignalIdentifier> output_names, std::string top_module_name) : m_context(std::move(ctx)), m_solver(std::move(solver)), m_smt_signal_name_to_expr_id(std::move(smt_signal_name_to_edge_id)), m_smt_timesteps(timesteps), m_signal_expr_vector(std::move(ev_edges)), m_input_signals(std::move(input_names)), m_output_signals(std::move(output_names)), m_top_module_name(std::move(top_module_name)) {
    m_timestamps.reserve(gsl::narrow<size_t>(m_smt_timesteps));
    for (SignalTime time = 0; time < m_smt_timesteps; time++) {
      m_timestamps.push_back(time);
    }
  }

  [[nodiscard]] SignalTime get_smt_timesteps() const {
    return m_smt_timesteps;
  }

  [[nodiscard]] uint64_t get_size() const override {
    return m_timestamps.size();
  }

  [[nodiscard]] SignalInfo get_signal(const SignalIdentifier& /*signal*/) const override;
  [[nodiscard]] uint64_t get_signal_size(const SignalIdentifier& signal) const override;
  [[nodiscard]] SignalValue get_signal_value(std::string_view signal_name, const std::vector<std::string>& hierarchy, SignalTime timestep) const;
  [[nodiscard]] SignalValue get_signal_value(const SignalIdentifier& signal_id, SignalTime timestep) const override;
  //GF [[nodiscard]] SignalValue get_signal_value_as_vector(const SignalIdentifier& signal_id, SignalTime timestep) const override;
  [[nodiscard]] SignalValues get_signal_values(const SignalIdentifier& signal_id) const override;

  [[nodiscard]] std::unordered_map<std::string, SignalValues> get_input_values(std::vector<SignalIdentifier> input_ids) const override;

  [[nodiscard]] std::optional<bool> is_tag_propagated(SignalIdentifier& input_signal, SignalTime input_time, SignalIdentifier& target_signal, SignalTime target_time) override;
  void add_state_constraints();

  [[nodiscard]] const std::vector<SignalTime>& get_timestamps() const override;
  [[nodiscard]] std::vector<SignalInfo> get_signals() const override { throw std::runtime_error("Not implemented get_signals in SMTSignalsDataManager"); };
};

class VCDSignalsDataManager : public SignalsDataManager {
private:
  std::vector<std::shared_ptr<VCDFile>> m_vcd_data;
  std::vector<const VCDScope*> m_root_scopes;
  uint64_t m_number_of_simulations = 0;

public:
  VCDSignalsDataManager() = default;
  explicit VCDSignalsDataManager(const std::shared_ptr<VCDFile>& vcd_data, uint64_t number_of_simulations) : m_number_of_simulations(number_of_simulations) {
    m_vcd_data.emplace_back(vcd_data);
  }
  explicit VCDSignalsDataManager(const std::shared_ptr<VCDFile>& vcd_data, const VCDScope* root_scope) : m_number_of_simulations(1) {
    m_vcd_data.emplace_back(vcd_data);
    m_root_scopes.emplace_back(root_scope);
  }
  explicit VCDSignalsDataManager(const std::vector<std::shared_ptr<VCDFile>>& vcd_data, std::vector<const VCDScope*> root_scopes, uint64_t number_of_simulations) : m_vcd_data(vcd_data), m_root_scopes(std::move(root_scopes)), m_number_of_simulations(number_of_simulations) {
  }

  [[nodiscard]] uint64_t get_size() const override { return m_number_of_simulations; };

  [[nodiscard]] uint64_t get_vcd_data_size() const override { return m_vcd_data.size(); }
  [[nodiscard]] Stepsize get_stepsize(const std::string& clk_name) const override {
    return Stepsize{m_vcd_data[0], clk_name};
  }

  [[nodiscard]] const std::vector<SignalTime>& get_timestamps() const override;
  [[nodiscard]] std::vector<SignalInfo> get_signals() const override;


  [[nodiscard]] uint32_t get_tag_count_signal(const std::string& signal_name, const std::vector<std::string>& signal_hierarchy) const override;
  [[nodiscard]] uint32_t get_tag_count_signal_to_signal(const SignalIdentifier& source_signal, const SignalIdentifier& target_signal) const override;

  [[nodiscard]] bool is_empty() const override {
    return m_vcd_data.empty();
  }

  [[nodiscard]] SignalInfo get_signal(const SignalIdentifier& signal) const override;
  [[nodiscard]] uint64_t get_signal_size(const SignalIdentifier& signal) const override;
  [[nodiscard]] SignalValues get_signal_values(const SignalIdentifier& signal_id) const override;
  [[nodiscard]] std::unordered_map<std::string, SignalValues> get_input_values(std::vector<SignalIdentifier> input_ids) const override;

  [[nodiscard]] SignalValue get_signal_value(const SignalIdentifier& signal_id, SignalTime timestep) const override;

  [[nodiscard]] std::vector<SignalValues> get_tag_values(const SignalIdentifier& signal_id) const override;
  [[nodiscard]] TagSourceList get_information_flow_source_signals_for_target_signal_at_timestep(const std::vector<SignalIdentifier>& source_signals, const SignalIdentifier& target_signal, SignalTime timestep) const override;
  [[nodiscard]] TagSourceList get_information_flow_source_signals_for_target_signal(const std::vector<SignalIdentifier>& source_signals, const SignalIdentifier& target_signal) const override;
  [[nodiscard]] std::optional<bool> is_tag_propagated(SignalIdentifier& input_signal, SignalTime input_time, SignalIdentifier& target_signal, SignalTime target_time) override;

private:
  [[nodiscard]] bool bitwise_vcd_compare(VCDBitVector& vec1, VCDBitVector& vec2) const override;
  [[nodiscard]] std::unordered_map<SignalIdentifier, VCDSignal*> get_source_tag_signals(uint64_t /*index*/, const std::vector<SignalIdentifier>& /*source_id*/) const;

  [[nodiscard]] VCDSignal* get_vcd_signal_ptr(const SignalIdentifier& signal) const;
  [[nodiscard]] VCDSignal* get_vcd_tag_signal_ptr(const SignalIdentifier& signal, const uint32_t& /*vcd_index*/) const;
};


}// namespace ducode
