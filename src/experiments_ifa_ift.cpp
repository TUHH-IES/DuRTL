/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>
#include <ducode/utility/smt.hpp>// NOLINT(misc-include-cleaner)

#include <CLI/App.hpp>
#include <CLI/Validators.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTypes.hpp>
#include <z3++.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {
auto create_design_instance(auto& design_folder_name, auto& design_name, auto& clk_name) {
  // setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / design_folder_name / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_folder_name / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_folder_name / fmt::format("{}.json", design_name);
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";

  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name(clk_name);

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // export ift enhanced verilog design file
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  design.write_verilog(exported_verilog, params);

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  assert(top_module != design.get_modules().end());

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clk_name));
  auto timesteps_per_simulation_run = tag_size / top_module->get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run,
                                                                      vcd_data);

  //exporting testbench with IFT
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(
      std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));
  testbench.write_verilog(exported_testbench, params);

  auto exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);
  auto tags_times = ducode::get_tags_times(vcd_data, gsl::narrow<uint32_t>(stepsize));
  auto exported_tags_vec = ducode::get_exported_tags_vec(exported_tags_files, number_simulation_runs,
                                                         tags_times, timesteps_per_simulation_run);
  testbench.write_tags(exported_tags_vec, params);

  auto instance = ducode::DesignInstance::create_instance(design);

  constexpr uint32_t sim_max_duration = 60;
  const auto max_time = std::chrono::system_clock::now() + std::chrono::minutes(sim_max_duration);
  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));
  return instance;
}

// get the number of information flows (each input/output pairs once) in the design instance
uint32_t get_IFT_IF_IO_count(ducode::DesignInstance& instance) {
  auto ifs = get_ift_information_flows(instance);
  return ducode::get_IF_count(ifs);
}

uint32_t get_IFT_IF_count_total(ducode::DesignInstance& instance) {
  auto ifs = get_ift_information_flows_complete(instance);
  return ducode::get_IF_count(ifs);
}

nlohmann::json get_IFT_IF_count(ducode::DesignInstance& instance) {
  nlohmann::json ift_res;
  auto ifs = get_ift_information_flows_complete(instance);
  ift_res["ift_if_count"] = ducode::get_IF_count(ifs);
  for (auto& output: ifs) {
    nlohmann::json output_json;
    std::unordered_map<std::string, ducode::SignalTime> ift_timesteps;// input -> ift_timesteps
    for (const auto& input: output.second) {
      if (ift_timesteps.contains(input.input_id.name)) {
        ift_timesteps[input.input_id.name] = std::min(ift_timesteps[input.input_id.name],
                                                      input.tag_observation_time - input.tag_injection_time);
      } else {
        ift_timesteps[input.input_id.name] = input.tag_observation_time - input.tag_injection_time;
      }
    }
    for (auto& entry: ift_timesteps) {
      nlohmann::json input_json;
      input_json["input"] = entry.first;
      input_json["ift_timesteps"] = entry.second;
      output_json.emplace_back(input_json);
    }
    ift_res[output.first] = output_json;
  };

  return ift_res;
}

uint32_t validate_IFT_IFs(ducode::DesignInstance& instance, auto& unroll_factor, auto& solver, auto& ctx,
                          auto& ev_edges_1, auto& ev_edges_2, auto& signal_name_to_expr_id_1,
                          auto& signal_name_to_expr_id_2) {
  const auto& design = instance.get_design();
  // get top module name
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  assert(top_module != design.get_modules().end());

  // create input constraints of the form (input1 != input2)
  auto input_names = top_module->get_input_names();
  z3::expr_vector input_constraints(ctx);
  std::unordered_map<std::string, uint32_t> input_to_constraint_id;
  for (uint32_t i = 0; i < input_names.size(); i++) {
    z3::expr input_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), input_names[i], 0)]];
    z3::expr input_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), input_names[i], 0)]];
    input_constraints.push_back(input_constraint_design_instance_1 != input_constraint_design_instance_2);
    input_to_constraint_id[input_names[i]] = i;
  }
  // create output constraints of the form (output1 != output2)
  auto output_names = top_module->get_output_names();
  std::vector<z3::expr_vector> output_constraints;
  std::unordered_map<std::string, uint32_t> output_to_constraint_id;
  for (uint32_t unroll_factor_cnt = 0; unroll_factor_cnt < unroll_factor; unroll_factor_cnt++) {
    z3::expr_vector constraints(ctx);
    for (uint32_t i = 0; i < output_names.size(); i++) {
      z3::expr output_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), output_names[i], unroll_factor_cnt)]];
      z3::expr output_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), output_names[i], unroll_factor_cnt)]];
      constraints.push_back(output_constraint_design_instance_1 != output_constraint_design_instance_2);
      output_to_constraint_id[output_names[i]] = i;
    }
    output_constraints.push_back(constraints);
  }

  // needed for z3::pbeq; set of coefficients, one for each input
  std::vector<int> val;
  for (uint32_t i = 0; i < input_names.size(); i++) { val.emplace_back(1); }
  uint32_t smt_ifs_count = 0;
  // going through all information flows and setting the constraints for the inputs and outputs and checking if the constraints are satisfiable => information flow confirmed/valid
  auto ifs = get_ift_information_flows(instance);
  for (const auto& if_input_list: ifs) {
    for (const auto& if_input: if_input_list.second) {
      solver.push();
      //setting the constraint for the input given by the tag
      solver.add(input_constraints[input_to_constraint_id[if_input]] == ctx.bool_val(true));
      for (uint32_t unroll_factor_cnt = 0; unroll_factor_cnt < unroll_factor; unroll_factor_cnt++) {
        solver.push();
        // setting the constraint for the current output
        solver.add(
            output_constraints[unroll_factor_cnt][output_to_constraint_id[if_input_list.first]] == ctx.bool_val(true));
        // using the input corresponding to the tag as the ExactlyOneOf constraint of the inputs
        solver.add(z3::pbeq(input_constraints, val.data(), 1));
        // as 2nd step adding the tag input constraint as true;
        if (solver.check() == z3::sat) {
          smt_ifs_count++;
          solver.pop();
          break;
        }
        solver.pop();
      }
      solver.pop();
    }
  }
  return smt_ifs_count;
}

uint32_t validate_IFT_IFs_under_assignments(ducode::DesignInstance& instance) {
  const auto& design = instance.get_design();
  // get top module name
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  assert(top_module != design.get_modules().end());

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges_1(ctx);
  z3::expr_vector ev_edges_2(ctx);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id_1;
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id_2;

  auto ifs = get_ift_information_flows_complete(instance);

  uint32_t smt_ifs_count = 0;
  ducode::IFT_info_map validated_ifs;

  auto timeout_counter = std::chrono::high_resolution_clock::now();
  bool timeout = false;

  for (auto& in_flow: ifs) {
    for (const auto& if_input: in_flow.second) {
      auto target_output_name = in_flow.first;
      auto target_input_name = if_input.input_id.name;
      auto tag_observation_time = if_input.tag_observation_time;
      auto tag_injection_time = if_input.tag_injection_time;

      if (tag_observation_time < tag_injection_time) { throw std::runtime_error("tag_observation_time < tag_injection_time"); }

      uint32_t max_unroll_factor = 1;
      if (tag_observation_time != tag_injection_time) {
        ducode::Stepsize step_size = instance.m_signal_tag_map->get_stepsize(design.get_clk_signal_name());
        max_unroll_factor = std::ceil((tag_observation_time - tag_injection_time) / step_size.value());
        max_unroll_factor = std::max(max_unroll_factor, step_size.value());
      } else {
        max_unroll_factor = 1;
      }
      solver.push();

      for (uint32_t unroll_factor = 0; unroll_factor < max_unroll_factor; unroll_factor++) {
        instance.export_smt2(ctx, solver, ev_edges_1, signal_name_to_expr_id_1, 1, unroll_factor);
        instance.export_smt2(ctx, solver, ev_edges_2, signal_name_to_expr_id_2, 2, unroll_factor);
      }

      // create input constraints of the form (input1 != input2)
      auto input_names = top_module->get_input_names();
      z3::expr_vector input_constraints(ctx);
      std::unordered_map<std::string, uint32_t> input_to_constraint_id;
      for (uint32_t i = 0; i < input_names.size(); i++) {
        z3::expr input_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), input_names[i], 0)]];
        z3::expr input_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), input_names[i], 0)]];
        input_constraints.push_back(input_constraint_design_instance_1 != input_constraint_design_instance_2);
        input_to_constraint_id[input_names[i]] = i;
      }

      std::vector<std::string> ff_names = get_all_flipflop_names(instance);
      // uint32_t initial_timestep = 0;
      z3::expr_vector state_constraints(ctx);
      std::unordered_map<std::string, uint32_t> state_signal_to_constraint_id;
      for (uint32_t i = 0; i < ff_names.size(); i++) {
        z3::expr state_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), ff_names[i], 0)]];
        z3::expr state_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), ff_names[i], 0)]];
        state_constraints.push_back(state_constraint_design_instance_1 == state_constraint_design_instance_2);
        state_signal_to_constraint_id[ff_names[i]] = i;
      }


      // create output constraints of the form (output1 != output2)
      auto output_names = top_module->get_output_names();
      std::vector<z3::expr_vector> output_constraints;
      std::unordered_map<std::string, uint32_t> output_to_constraint_id;
      for (uint32_t unroll_factor = 0; unroll_factor < max_unroll_factor; unroll_factor++) {
        z3::expr_vector constraints(ctx);
        for (uint32_t i = 0; i < output_names.size(); i++) {
          z3::expr output_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), output_names[i], unroll_factor)]];
          z3::expr output_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), output_names[i], unroll_factor)]];
          constraints.push_back(output_constraint_design_instance_1 != output_constraint_design_instance_2);
          output_to_constraint_id[output_names[i]] = i;
        }
        output_constraints.push_back(constraints);
      }

      // needed for z3::pbeq; set of coefficients, one for each input
      std::vector<int> val;
      val.reserve(input_names.size());
      for (uint32_t i = 0; i < input_names.size(); i++) {
        val.emplace_back(1);
      }

      for (uint32_t timestep = 0; timestep < max_unroll_factor; timestep++) {
        solver.push();

        // instead of adding output values as constraints -> only add input_values and let SMT create a Model -> compare the SMT model values with simulation output values
        for (auto& input_name: input_names) {
          if (input_name == target_input_name) { continue; }
          add_smt_signal_value(input_name, instance, solver, ctx, ev_edges_1, timestep, signal_name_to_expr_id_1, 1);
          //timestep needs to be multiplied by clockcycle length
          add_smt_signal_value(input_name, instance, solver, ctx, ev_edges_2, timestep, signal_name_to_expr_id_2, 2);
          //timestep needs to be multiplied by clockcycle length
        }

        solver.pop();
      }

      //setting the constraint for the input given by the tag
      solver.add(input_constraints[input_to_constraint_id[target_input_name]] == ctx.bool_val(true));
      // using the input corresponding to the tag as the ExactlyOneOf constraint of the inputs
      solver.add(z3::pbeq(input_constraints, val.data(), 1));
      // setting the constraint for the current output
      solver.add(
          output_constraints[gsl::narrow<uint64_t>(max_unroll_factor - 1)][output_to_constraint_id[target_output_name]] == ctx.bool_val(true));

      // as 2nd step adding the tag input constraint as true;
      // if (solver.check() == z3::sat) {
      //   smt_ifs_count++;
      // }

      solver.pop();

      const uint32_t timeout_minutes = 60;
      auto timeout_counter_end = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::minutes>(timeout_counter_end - timeout_counter).count() >
          timeout_minutes) {
        timeout = true;
        break;
      }
    }
    if (timeout) { break; }
  }
  return smt_ifs_count;
}

nlohmann::json get_IFA_IF_count(ducode::DesignInstance& instance, auto& solver, auto& ctx,
                                auto& unroll_factor_threshold, auto& ev_edges_1, auto& ev_edges_2,
                                auto& signal_name_to_expr_id_1, auto& signal_name_to_expr_id_2) {
  nlohmann::json ifa_results;
  const auto& design = instance.get_design();
  const auto max_number_io_pairs = design.get_module(design.get_top_module_name()).get_input_names().size() * design.get_module(design.get_top_module_name()).get_output_names().size();

  // get top module name
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  assert(top_module != design.get_modules().end());

  instance.export_smt2(ctx, solver, ev_edges_1, signal_name_to_expr_id_1, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges_2, signal_name_to_expr_id_2, 2, 0);

  // create input constraints of the form (input1 != input2)
  auto input_names = top_module->get_input_names();
  z3::expr_vector input_constraints(ctx);
  std::unordered_map<std::string, uint32_t> input_to_constraint_id;
  for (uint32_t i = 0; i < input_names.size(); i++) {
    z3::expr input_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), input_names[i], 0)]];
    z3::expr input_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), input_names[i], 0)]];
    input_constraints.push_back(input_constraint_design_instance_1 != input_constraint_design_instance_2);
    input_to_constraint_id[input_names[i]] = i;
  }

  std::vector<std::string> ff_names = get_all_flipflop_names(instance);
  // uint32_t initial_timestep = 0;
  z3::expr_vector state_constraints(ctx);
  std::unordered_map<std::string, uint32_t> state_signal_to_constraint_id;
  for (uint32_t i = 0; i < ff_names.size(); i++) {
    z3::expr state_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), ff_names[i], 0)]];
    z3::expr state_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), ff_names[i], 0)]];
    state_constraints.push_back(state_constraint_design_instance_1 == state_constraint_design_instance_2);
    state_signal_to_constraint_id[ff_names[i]] = i;
  }

  // create output constraints of the form (output1 != output2)
  auto output_names = top_module->get_output_names();
  std::vector<z3::expr_vector> output_constraints;
  std::unordered_map<std::string, uint32_t> output_to_constraint_id;
  z3::expr_vector constraints_0(ctx);
  for (uint32_t i = 0; i < output_names.size(); i++) {
    z3::expr output_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), output_names[i], 0)]];
    z3::expr output_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), output_names[i], 0)]];
    constraints_0.push_back(output_constraint_design_instance_1 != output_constraint_design_instance_2);
    output_to_constraint_id[output_names[i]] = i;
  }
  output_constraints.push_back(constraints_0);

  // maps input(key) to set of outputs(value) that are influenced by the input
  std::unordered_map<std::string, std::unordered_set<std::string>> IFs_map;
  uint64_t ifa_if_counter = 0;
  std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> ifa_results_map;
  // input -> (output -> # of unrolling steps)

  for (uint32_t unroll_factor = 0; unroll_factor <= unroll_factor_threshold; unroll_factor++) {
    ifa_if_counter = 0;
    if (unroll_factor != 0) {
      instance.export_smt2(ctx, solver, ev_edges_1, signal_name_to_expr_id_1, 1, unroll_factor);
      instance.export_smt2(ctx, solver, ev_edges_2, signal_name_to_expr_id_2, 2, unroll_factor);

      z3::expr_vector constraints(ctx);
      for (uint32_t i = 0; i < output_names.size(); i++) {
        z3::expr output_constraint_design_instance_1 = ev_edges_1[signal_name_to_expr_id_1[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), output_names[i], unroll_factor)]];
        z3::expr output_constraint_design_instance_2 = ev_edges_2[signal_name_to_expr_id_2[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), output_names[i], unroll_factor)]];
        constraints.push_back(output_constraint_design_instance_1 != output_constraint_design_instance_2);
        output_to_constraint_id[output_names[i]] = i;
      }
      output_constraints.push_back(constraints);
    }
    // needed for z3::pbeq; set of coefficients, one for each input
    std::vector<int> val;
    val.reserve(input_names.size());
    for (uint32_t i = 0; i < input_names.size(); i++) {
      val.emplace_back(1);
    }

    // store current solver state
    solver.push();
    // ExactlyOneOf(B)
    solver.add(z3::pbeq(input_constraints, val.data(), 1));//pseudo-boolean exactly one of

    // AtLeastOneOf(C) {
    z3::expr_vector or_constraints(ctx);
    for (const auto& output_constraint: output_constraints) { or_constraints.push_back(z3::mk_or(output_constraint)); }
    solver.add(z3::mk_and(state_constraints));
    solver.add(or_constraints.back() == ctx.bool_val(true));
    // } AtLeastOneOf(C)

    for (const auto& name: input_names) {
      // std::unordered_map<std::string, uint32_t> ifa_results_map;// input -> # of unrolling steps

      if (name == design.get_clk_signal_name()) { continue; }
      std::unordered_set<std::string> IF_outputs;

      solver.push();
      solver.add(input_constraints[input_to_constraint_id[name]] == ctx.bool_val(true));
      for (const auto& in_name: input_names) {
        if (in_name != name) { solver.add(input_constraints[input_to_constraint_id[in_name]] == ctx.bool_val(false)); }
      }// end of input_names loop
      for (const auto& output: output_names) {
        // for (const auto& output_constraint: output_constraints) {
        if (IFs_map.contains(name)) {
          if (IFs_map[name].contains(output)) { continue; }
        }
        spdlog::info("Test IFA for input: {} + output: {}", name, output);
        solver.push();
        solver.add(output_constraints.back()[output_to_constraint_id[output]] == ctx.bool_val(true));
        if (solver.check() == z3::sat) {
          if (ifa_results_map.contains(name)) {
            if (ifa_results_map[name].contains(output)) {
              if (ifa_results_map[name][output] > unroll_factor) { ifa_results_map[name][output] = unroll_factor; }
            } else {
              ifa_results_map[name][output] = unroll_factor;
            }
          } else {
            ifa_results_map[name][output] = unroll_factor;
          }
          IF_outputs.insert(output);
        } else {
          spdlog::info("unsat");
        }
        solver.pop();
        // }
      }// end of output_names loop
      if (IFs_map.contains(name)) {
        IFs_map[name].insert(IF_outputs.begin(), IF_outputs.end());
      } else {
        IFs_map.emplace(name, IF_outputs);
      }
      solver.pop();
      nlohmann::json ifa_res_map;
      for (auto& entry: ifa_results_map) {
        for (auto& entry2: entry.second) { ifa_res_map[entry2.first] = entry2.second; }
      }
      ifa_results[name] = ifa_res_map;
    }// end of input_names loop
    solver.pop();

    for (const auto& [input, outputs]: IFs_map) { ifa_if_counter += outputs.size(); }
    if (ifa_if_counter >= max_number_io_pairs) { break; }
  }// end of unroll_factor loop
  ifa_results["ifa_if_count"] = ifa_if_counter;
  return ifa_results;
}

void run_experiment_ifa(auto& unroll_factor_threshold, auto& design_folder_name, auto& design_name, auto& clk_name) {
  /// ideas for interface to remove tag_map call from tests
  /// params["tag_scheme"] = full/random;
  /// params["random_tag_number"] = 25%;
  /// params["stepsize"] = 1;

  nlohmann::json results;

  auto start_ift = std::chrono::high_resolution_clock::now();

  // create instance of the design
  auto instance = create_design_instance(design_folder_name, design_name, clk_name);
  auto& design = instance.get_design();
  // do ift analysis :: get number of IFT information flows
  uint32_t if_count = get_IFT_IF_IO_count(instance);
  spdlog::info("{}", if_count);
  auto end_ift = std::chrono::high_resolution_clock::now();

  nlohmann::json ift_res = get_IFT_IF_count(instance);

  // # of IO pairs ( remove the clk signal from # of inputs)
  results["# of IO pairs"] = (design.get_module(design.get_top_module_name()).get_input_names().size() - 1) * design.get_module(design.get_top_module_name()).get_output_names().size();
  results["IFs identified by IFT (IO pairs)"] = if_count;
  results["IFT runtime [s]"] = std::chrono::duration_cast<std::chrono::seconds>(end_ift - start_ift).count();
  results["IFT runtime [ms]"] = std::chrono::duration_cast<std::chrono::milliseconds>(end_ift - start_ift).count();
  results["IFT IFs"] = ift_res;
  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges_1(ctx);
  z3::expr_vector ev_edges_2(ctx);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id_1;
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id_2;

  nlohmann::json results_ifa;
  auto start_ifa = std::chrono::high_resolution_clock::now();
  // do IFA analysis :: get number of IFA information flows
  nlohmann::json ifa_if_counter = get_IFA_IF_count(instance, solver, ctx, unroll_factor_threshold, ev_edges_1,
                                                   ev_edges_2, signal_name_to_expr_id_1, signal_name_to_expr_id_2);
  auto end_ifa = std::chrono::high_resolution_clock::now();

  // results_ifa["IFs validated by SMT"] = smt_ifs_count;
  results_ifa["SMT runtime [s]"] = std::chrono::duration_cast<std::chrono::seconds>(end_ifa - start_ifa).count();

  results_ifa["IFA IFs identified"] = ifa_if_counter;
  results[fmt::format("UF:{}", unroll_factor_threshold)] = results_ifa;

  std::fstream os_json(fmt::format("results/results_IFA_{}_{}.json", design_name, unroll_factor_threshold),
                       std::ios::out);
  os_json << results;
  os_json.close();
}

void run_experiment_ift_validation(auto& design_folder_name, auto& design_name, auto& clk_name) {
  /// ideas for interface to remove tag_map call from tests
  /// params["tag_scheme"] = full/random;
  /// params["random_tag_number"] = 25%;
  /// params["stepsize"] = 1;

  nlohmann::json results;

  auto start_ift = std::chrono::high_resolution_clock::now();

  // create instance of the design
  auto instance = create_design_instance(design_folder_name, design_name, clk_name);
  auto& design = instance.get_design();
  // do ift analysis :: get number of IFT information flows
  uint32_t if_count = get_IFT_IF_IO_count(instance);
  spdlog::info("{}", if_count);

  auto end_ift = std::chrono::high_resolution_clock::now();

  // # of IO pairs ( remove the clk signal from # of inputs)
  results["# of IO pairs"] = (design.get_module(design.get_top_module_name()).get_input_names().size() - 1) * design.get_module(design.get_top_module_name()).get_output_names().size();
  results["IFs identified by IFT (IO pairs)"] = if_count;
  results["IFT runtime [s]"] = std::chrono::duration_cast<std::chrono::seconds>(end_ift - start_ift).count();
  results["IFT runtime [ms]"] = std::chrono::duration_cast<std::chrono::milliseconds>(end_ift - start_ift).count();

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges_1(ctx);
  z3::expr_vector ev_edges_2(ctx);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id_1;
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id_2;

  auto start_ifa_validation = std::chrono::high_resolution_clock::now();
  //validate the IFT information flows by using IFA under assignments from the simulation data
  auto counter = validate_IFT_IFs_under_assignments(instance);
  // counter is the number of IFT information flows that are validated by IFA (currently NOT under assignments)
  results["IFT IFs (specific assignments)"] = get_IFT_IF_count_total(instance);
  results["IFT IFs validated by IFA"] = counter;
  auto end_ifa_validation = std::chrono::high_resolution_clock::now();
  results["IFT-IFA validation runtime [s]"] = std::chrono::duration_cast<std::chrono::seconds>(
                                                  end_ifa_validation - start_ifa_validation)
                                                  .count();

  std::fstream os_json(fmt::format("results/results_ift_val_10_{}.json", design_name), std::ios::out);
  os_json << results;
  os_json.close();
}
}// namespace

int main(int argc, char* argv[]) {
  // global setup...

  CLI::App app{"Basic IFT execution"};

  std::string design_name;
  std::string design_folder_name;
  std::string clk_name;
  uint32_t unroll_factor = 0;
  uint32_t mode = 0;
  app.add_option("-d,--design", design_name, "The design")->required();
  app.add_option("-f,--folder", design_folder_name, "The folder containing the design")->required();
  app.add_option("-c,--clk", clk_name, "The clk")->required();
  app.add_option("-u,--unroll_factor", unroll_factor, "unroll_factor for SMT solver")->required()->check(CLI::PositiveNumber)->default_val(1);
  app.add_option("-m,--mode", mode, "The Mode")->required();

  CLI11_PARSE(app, argc, argv);

  //making two sinks, one for consol, one for log file
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::info);
  console_sink->set_pattern("%^[%l] %s:%# %! %v");

  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("tester.log", true);
  file_sink->set_level(spdlog::level::trace);
  file_sink->set_pattern("%^[%l] %s:%# %! %v");
  spdlog::sinks_init_list sink_list = {file_sink, console_sink};

  //registering the logger so it stays available for testcases
  spdlog::logger logger("duLog", sink_list.begin(), sink_list.end());
  logger.set_level(spdlog::level::trace);
  spdlog::register_logger(std::make_shared<spdlog::logger>(logger));

  //setting the registered logger to default
  auto log = spdlog::get("duLog");
  spdlog::set_default_logger(log);

  spdlog::debug("Configured LOGGER");

  if (mode == 1) {
    run_experiment_ifa(unroll_factor, design_folder_name, design_name, clk_name);
  } else if (mode == 2) {
    // reduce the number of IFT traces that are validated by IFA to a maximum number of unrolling steps
    run_experiment_ift_validation(design_folder_name, design_name, clk_name);
  }

  spdlog::shutdown();

  return 0;
}
