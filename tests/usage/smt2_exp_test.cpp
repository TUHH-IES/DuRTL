#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/utility/smt.hpp>// NOLINT(misc-include-cleaner)

#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <z3++.h>

#include <cstdint>
#include <fstream>
#include <ios>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <z3_api.h>

// example to get all possible assignments of inputs
TEST_CASE("smt_and_exp_test", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "and" / "and.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);
  nlohmann::json results;

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;// maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_set<std::string> input_names{"a", "b"};
  std::unordered_set<std::string> output_names{"c"};
  uint32_t inst_id = 2;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, inst_id);

  z3::expr_vector input_constraints(ctx);
  for (const auto& name: input_names) {
    input_constraints.push_back(ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), name, 0)]] != ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), name, 0)]]);
  }

  z3::expr_vector output_constraints(ctx);
  for (const auto& name: output_names) {
    output_constraints.push_back(ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), name, 0)]] != ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), name, 0)]]);
  }
  spdlog::debug("{}", ev_edges);

  solver.push();
  // ExactlyOneOf(B)
  solver.add(z3::mk_xor(input_constraints) == ctx.bool_val(true));
  // AtLeastOneOf(C)
  solver.add(z3::mk_or(output_constraints) == ctx.bool_val(true));
  CHECK(solver.check() == z3::sat);
  spdlog::info("{}", solver.to_smt2());
  // loop to check all possible assignments of inputs
  // stops when no more satisfiable assignments are possible
  uint32_t result_count = 1;
  while (solver.check() == z3::sat) {
    spdlog::debug("{}", solver.get_model());

    z3::expr_vector remove_assignment_vars(ctx);
    z3::expr remove_assignment(ctx);

    // loop to construct the expression to remove the current assignment from the solver
    for (const auto& in_constraint: input_constraints) {
      if (solver.get_model().eval(in_constraint).bool_value() == Z3_L_TRUE) {
        remove_assignment_vars.push_back(in_constraint);
        results[fmt::format("inputs_{}", result_count)].push_back(in_constraint.to_string());
      } else {
        remove_assignment_vars.push_back(!in_constraint);
        results[fmt::format("inputs_{}", result_count)].push_back((!in_constraint).to_string());
      }
      spdlog::debug("{}", solver.get_model().eval(in_constraint).bool_value() == Z3_L_TRUE ? "true" : "false");
    }
    remove_assignment = z3::mk_and(remove_assignment_vars);
    solver.add(remove_assignment == ctx.bool_val(false));
    spdlog::info("{}", solver.to_smt2());
    result_count++;
  }
  std::fstream os_json(fmt::format("results_IFA_test_{}.json", design.get_top_module_name()),
                       std::ios::out);
  os_json << results;
  os_json.close();
  solver.pop();
}

TEST_CASE("smt_dffdesign", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "dff_design" / "dff_design.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);
  nlohmann::json results;

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;// maps input and output names to smt signal ids in ev_edges (id == index)
  std::vector<std::string> input_names = design.get_top_module().get_input_names();
  std::vector<std::string> output_names = design.get_top_module().get_output_names();
  uint32_t inst_id = 2;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, inst_id);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, inst_id, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, inst_id, 2);

  z3::expr_vector input_constraints(ctx);
  for (const auto& name: input_names) {
    for (uint32_t unroll_step = 0; unroll_step < 3; unroll_step++) {
      input_constraints.push_back(ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), name, unroll_step)]] != ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), name, unroll_step)]]);
    }
  }

  z3::expr_vector output_constraints(ctx);
  for (const auto& name: output_names) {
    for (uint32_t unroll_step = 0; unroll_step < 3; unroll_step++) {
      output_constraints.push_back(ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), name, unroll_step)]] != ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), name, unroll_step)]]);
    }
  }

  std::vector<std::string> ff_names = get_all_flipflop_names(instance);
  // uint32_t initial_timestep = 0;
  z3::expr_vector state_constraints(ctx);
  std::unordered_map<std::string, uint32_t> state_signal_to_constraint_id;
  for (uint32_t i = 0; i < ff_names.size(); i++) {
    z3::expr state_constraint_design_instance_1 = ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), ff_names[i], 0)]];
    z3::expr state_constraint_design_instance_2 = ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), ff_names[i], 0)]];
    state_constraints.push_back(state_constraint_design_instance_1 == state_constraint_design_instance_2);
    state_signal_to_constraint_id[ff_names[i]] = i;
  }

  spdlog::debug("{}", ev_edges);

  // needed for z3::pbeq; set of coefficients, one for each input constraint
  std::vector<int> val;
  val.reserve(input_constraints.size());
  for (uint32_t i = 0; i < input_constraints.size(); i++) {
    val.emplace_back(1);
  }

  // store current solver state
  solver.push();
  // ExactlyOneOf(B)
  solver.add(z3::pbeq(input_constraints, val.data(), 1));//pseudo-boolean exactly one of
  // AtLeastOneOf(C)
  solver.add(z3::mk_or(output_constraints) == ctx.bool_val(true));
  // StateConstraints: states in unrolling step 0 have to be equal
  solver.add(z3::mk_and(state_constraints));
  CHECK(solver.check() == z3::sat);
  spdlog::info("{}", solver.to_smt2());
  // loop to check all possible assignments of inputs
  // stops when no more satisfiable assignments are possible
  uint32_t result_count = 1;
  while (solver.check() == z3::sat) {
    spdlog::debug("{}", solver.get_model());

    z3::expr_vector remove_assignment_vars(ctx);
    z3::expr remove_assignment(ctx);

    // loop to construct the expression to remove the current assignment from the solver
    for (const auto& in_constraint: input_constraints) {
      if (solver.get_model().eval(in_constraint).bool_value() == Z3_L_TRUE) {
        remove_assignment_vars.push_back(in_constraint);
        results[fmt::format("inputs_{}", result_count)].push_back(in_constraint.to_string());
      } else {
        remove_assignment_vars.push_back(!in_constraint);
        results[fmt::format("inputs_{}", result_count)].push_back((!in_constraint).to_string());
      }
      spdlog::debug("{}", solver.get_model().eval(in_constraint).bool_value() == Z3_L_TRUE ? "true" : "false");
    }
    for (const auto& out_constraint: output_constraints) {
      if (solver.get_model().eval(out_constraint).bool_value() == Z3_L_TRUE) {
        results[fmt::format("outputs_{}", result_count)].push_back(out_constraint.to_string());
      } else {
        results[fmt::format("outputs_{}", result_count)].push_back((!out_constraint).to_string());
      }
    }
    remove_assignment = z3::mk_and(remove_assignment_vars);
    solver.add(remove_assignment == ctx.bool_val(false));
    spdlog::info("{}", solver.to_smt2());
    result_count++;
    std::fstream os_json(fmt::format("results_IFA_test_{}.json", design.get_top_module_name()),
                         std::ios::out);
    os_json << results;
    os_json.close();
  }
  std::fstream os_json(fmt::format("results_IFA_test_{}.json", design.get_top_module_name()),
                       std::ios::out);
  os_json << results;
  os_json.close();
  solver.pop();
}

/// tests for SMT experiment functionalities
TEST_CASE("smt_SPI_Master", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Master" / "SPI_Master.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);
  nlohmann::json results;

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;// maps input and output names to smt signal ids in ev_edges (id == index)
  std::vector<std::string> input_names = design.get_top_module().get_input_names();
  std::vector<std::string> output_names = design.get_top_module().get_output_names();
  uint32_t inst_id = 2;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, inst_id);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, inst_id, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, inst_id, 2);

  z3::expr_vector input_constraints(ctx);
  for (const auto& name: input_names) {
    for (uint32_t unroll_step = 0; unroll_step < 3; unroll_step++) {
      input_constraints.push_back(ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), name, unroll_step)]] != ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), name, unroll_step)]]);
    }
  }

  z3::expr_vector output_constraints(ctx);
  for (const auto& name: output_names) {
    for (uint32_t unroll_step = 0; unroll_step < 3; unroll_step++) {
      output_constraints.push_back(ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), name, unroll_step)]] != ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), name, unroll_step)]]);
    }
  }

  std::vector<std::string> ff_names = get_all_flipflop_names(instance);
  // uint32_t initial_timestep = 0;
  z3::expr_vector state_constraints(ctx);
  std::unordered_map<std::string, uint32_t> state_signal_to_constraint_id;
  for (uint32_t i = 0; i < ff_names.size(); i++) {
    z3::expr state_constraint_design_instance_1 = ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(1, std::vector<std::string>(1, design.get_top_module_name()), ff_names[i], 0)]];
    z3::expr state_constraint_design_instance_2 = ev_edges[signal_name_to_expr_id[ducode::create_smt_signal_name(2, std::vector<std::string>(1, design.get_top_module_name()), ff_names[i], 0)]];
    state_constraints.push_back(state_constraint_design_instance_1 == state_constraint_design_instance_2);
    state_signal_to_constraint_id[ff_names[i]] = i;
  }

  spdlog::debug("{}", ev_edges);

  // needed for z3::pbeq; set of coefficients, one for each input constraint
  std::vector<int> val;
  val.reserve(input_constraints.size());
  for (uint32_t i = 0; i < input_constraints.size(); i++) {
    val.emplace_back(1);
  }

  // store current solver state
  solver.push();
  // ExactlyOneOf(B)
  solver.add(z3::pbeq(input_constraints, val.data(), 1));//pseudo-boolean exactly one of
  // AtLeastOneOf(C)
  solver.add(z3::mk_or(output_constraints) == ctx.bool_val(true));
  // StateConstraints: states in unrolling step 0 have to be equal
  solver.add(z3::mk_and(state_constraints));
  CHECK(solver.check() == z3::sat);
  spdlog::info("{}", solver.to_smt2());
  // loop to check all possible assignments of inputs
  // stops when no more satisfiable assignments are possible
  uint32_t result_count = 1;
  while (solver.check() == z3::sat) {
    spdlog::debug("{}", solver.get_model());

    z3::expr_vector remove_assignment_vars(ctx);
    z3::expr remove_assignment(ctx);

    // loop to construct the expression to remove the current assignment from the solver
    for (const auto& in_constraint: input_constraints) {
      if (solver.get_model().eval(in_constraint).bool_value() == Z3_L_TRUE) {
        remove_assignment_vars.push_back(in_constraint);
        results[fmt::format("inputs_{}", result_count)].push_back(in_constraint.to_string());
      } else {
        remove_assignment_vars.push_back(!in_constraint);
        results[fmt::format("inputs_{}", result_count)].push_back((!in_constraint).to_string());
      }
      spdlog::debug("{}", solver.get_model().eval(in_constraint).bool_value() == Z3_L_TRUE ? "true" : "false");
    }
    for (const auto& out_constraint: output_constraints) {
      if (solver.get_model().eval(out_constraint).bool_value() == Z3_L_TRUE) {
        results[fmt::format("outputs_{}", result_count)].push_back(out_constraint.to_string());
      } else {
        results[fmt::format("outputs_{}", result_count)].push_back((!out_constraint).to_string());
      }
    }
    remove_assignment = z3::mk_and(remove_assignment_vars);
    solver.add(remove_assignment == ctx.bool_val(false));
    spdlog::info("{}", solver.to_smt2());
    result_count++;
    std::fstream os_json(fmt::format("results_IFA_test_{}.json", design.get_top_module_name()),
                         std::ios::out);
    os_json << results;
    os_json.close();
  }
  std::fstream os_json(fmt::format("results_IFA_test_{}.json", design.get_top_module_name()),
                       std::ios::out);
  os_json << results;
  os_json.close();
  solver.pop();
}

TEST_CASE("wind_turbine_shutdown", "[smt2_unroll][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "wind_turbin" / "wind_turbin.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //instance.write_verilog("wind_turbine.v");
  //instance.write_graphviz("wind_turbine.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  uint32_t unroll_factor = 2;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  uint32_t action_id = signal_name_to_edge_id["action"];
  uint32_t wind_speed_id = signal_name_to_edge_id["wind_speed"];
  uint32_t vibration_level_id = signal_name_to_edge_id["vibration_level"];

  z3::expr_vector signals(ctx);
  signals.push_back(ev_edges[action_id]);
  signals.push_back(ev_edges[wind_speed_id]);
  signals.push_back(ev_edges[vibration_level_id]);

  const int32_t max_windspeed = 150;
  const int32_t bit_width_1 = 8;
  const int32_t bit_width_2 = 7;
  const int32_t max_vibration_level = 100;
  const int32_t shutdown_act = 3;


  z3::expr wind_speed_threshold = ctx.bv_val(max_windspeed, bit_width_1);
  z3::expr vibration_level_threshold = ctx.bv_val(max_vibration_level, bit_width_1);
  z3::expr shutdown_action = ctx.bv_val(shutdown_act, bit_width_1);

  z3::expr wind_speed_signal = z3::zext(signals[1], bit_width_2);
  z3::expr vibration_level_signal = z3::zext(signals[2], bit_width_2);

  // Case 1: Isolate Wind Speed Condition
  solver.push();
  solver.add(wind_speed_signal >= wind_speed_threshold);
  solver.add(z3::zext(signals[0], bit_width_2) == shutdown_action);

  if (solver.check() == z3::sat) {
    z3::model model = solver.get_model();
    spdlog::info("Shutdown caused by high wind speed:");
    spdlog::info("  Wind Speed: {} (Threshold: {})", model.eval(wind_speed_signal).as_int64(), wind_speed_threshold.as_int64());
  } else {
    spdlog::info("No shutdown due to wind speed alone.");
  }
  solver.pop();

  // Case 2: Isolate Vibration Level Condition
  solver.push();
  solver.add(vibration_level_signal >= vibration_level_threshold);
  solver.add(z3::zext(signals[0], bit_width_2) == shutdown_action);

  if (solver.check() == z3::sat) {
    z3::model model = solver.get_model();
    spdlog::info("Shutdown caused by high vibration level:");
    spdlog::info("  Vibration Level: {}  (Threshold: {})", model.eval(vibration_level_signal).as_int64(), vibration_level_threshold.as_int64());
  } else {
    spdlog::info("No shutdown due to vibration level alone.");
  }
  solver.pop();

  // Case 3: Combined Condition
  solver.push();
  solver.add((wind_speed_signal >= wind_speed_threshold) || (vibration_level_signal >= vibration_level_threshold));
  solver.add(z3::zext(signals[0], bit_width_2) == shutdown_action);

  if (solver.check() == z3::sat) {
    z3::model model = solver.get_model();
    spdlog::info("Shutdown caused by combined conditions:");
    spdlog::info("  Wind Speed: {} (Threshold: {})", model.eval(wind_speed_signal).as_int64(), wind_speed_threshold.as_int64());
    spdlog::info("  Vibration Level: {} (Threshold: {})", model.eval(vibration_level_signal).as_int64(), vibration_level_threshold.as_int64());
    spdlog::info("  Action Taken: Shutdown ({})", model.eval(signals[0]).as_int64());
  } else {
    spdlog::info("No shutdown even under combined conditions.");
  }
  solver.pop();
}
