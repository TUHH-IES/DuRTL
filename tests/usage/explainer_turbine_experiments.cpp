//
// Created by moham on 11/19/2025.
//
#include <common_definitions.hpp>
#include <ducode/ExpLib/Analysis.h>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>

#include <catch2/catch_test_macros.hpp>
#include <z3++.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


TEST_CASE("wind_turbine_7_states_explainer", "[Explainer]") {

  // Parse design and create an instance
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "turbine" / "turbine-7-states.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("turbine-7-states.v");
  instance.write_graphviz("turbine-7-states.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);

  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 7;

  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  constexpr uint8_t NORMAL_OPERATION = 0b00000000;
  constexpr uint8_t BLADE_ADJUSTMENT = 0b00000101;
  constexpr uint8_t SHUTDOWN = 0b00000111;
  constexpr uint8_t REDUCED_SPEED = 0b00000100;
  constexpr uint8_t WIND_SPEED_THRESHOLD = 0b01100100;
  constexpr uint8_t NORMAL_POWER = 0b00011000;
  constexpr unsigned BV_WIDTH_2 = 2;
  constexpr unsigned BV_WIDTH_5 = 5;
  constexpr unsigned BV_WIDTH_8 = 8;

  auto get_signal = [&](const std::string& name) {
    return ev_edges[signal_name_to_edge_id[name]];
  };

  z3::expr wind_speed = get_signal("|D1_ComposedSystem_wind_speed_time_6|");
  z3::expr power = get_signal("|D1_ComposedSystem_power_time_6|");
  z3::expr power_need = get_signal("|D1_ComposedSystem_power_need_time_6|");
  z3::expr park_command = get_signal("|D1_ComposedSystem_park_command_time_6|");
  z3::expr env_signal = get_signal("|D1_ComposedSystem_turbine_env_time_6|");
  z3::expr turbine_state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_6|");

  z3::expr state0 = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_0|");
  solver.add(state0 == ctx.bv_val(SHUTDOWN, BV_WIDTH_8));

  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };

  enum class Tag : std::uint8_t { A,
                                  E,
                                  C };
  auto tag_name = [](Tag t) {
    switch (t) {
      case Tag::A:
        return "ΓA";
      case Tag::E:
        return "ΓE";
      case Tag::C:
        return "ΓC";
    }
    return "Γ?";
  };

  struct Assumption {
    std::string name;
    Tag tag;
    int weight;
    z3::expr act;
    z3::expr prop;
    bool active;
  };

  std::vector<Assumption> gamma;
  z3::expr_vector active_acts(ctx);

  auto add_assumption = [&](Tag tag, int weight, const std::string& label, const z3::expr& formula) {
    z3::expr act = ctx.bool_const(label.c_str());
    solver.add(z3::implies(act, formula));
    gamma.push_back({label, tag, weight, act, formula, true});
    active_acts.push_back(act);
  };

  add_assumption(Tag::A, 1, "GA_initial_shutdown", state0 == bv8(SHUTDOWN));
  add_assumption(Tag::A, 1, "GA_final_normal_operation", turbine_state == bv8(NORMAL_OPERATION));

  add_assumption(Tag::E, 2, "GE_high_wind_speed", wind_speed > bv8(WIND_SPEED_THRESHOLD));
  add_assumption(Tag::A, 1, "GA_normal_power", power == bv8(NORMAL_POWER));
  add_assumption(Tag::A, 1, "GA_power_need_2", power_need == bv8(BV_WIDTH_2));
  add_assumption(Tag::A, 1, "GA_no_park_command", park_command == bv8(NORMAL_OPERATION));

  add_assumption(Tag::E, 2, "GE_high_wind_implies_blade_adjust",
                 z3::implies(wind_speed > bv8(WIND_SPEED_THRESHOLD), env_signal == bv8(BLADE_ADJUSTMENT)));
  add_assumption(Tag::E, 2, "GE_normal_power_implies_normal_env",
                 z3::implies(power == bv8(NORMAL_POWER), env_signal == bv8(NORMAL_OPERATION)));
  add_assumption(Tag::C, 0, "GC_power_need_low_implies_reduced_speed",
                 z3::implies(power_need < bv8(BV_WIDTH_5), env_signal == bv8(REDUCED_SPEED)));
  add_assumption(Tag::C, 0, "GC_env_normal_when_no_extreme_conditions",
                 env_signal == bv8(NORMAL_OPERATION));

  add_assumption(Tag::C, 0, "GC_env_blade_adj_implies_not_normal_op",
                 z3::implies(env_signal == bv8(BLADE_ADJUSTMENT), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_reduced_speed_implies_not_normal_op",
                 z3::implies(env_signal == bv8(REDUCED_SPEED), !(turbine_state == bv8(NORMAL_OPERATION))));

  auto rebuild_active_vector = [&]() {
    z3::expr_vector acts(ctx);
    for (auto& a: gamma) {
      if (a.active) {
        acts.push_back(a.act);
      }
    }
    return acts;
  };

  auto find_by_act_name = [&](const std::string& n) -> int {
    for (int i = 0; i < static_cast<int>(gamma.size()); ++i) {
      if (gamma[i].act.decl().name().str() == n) {
        return i;
      }
    }
    return -1;
  };

  struct ExplItem {
    std::string name;
    Tag tag;
    int weight;
  };
  std::vector<ExplItem> explanation;

  // ==============================================================
  // Scalability Metrics: start timer
  double cpu_start = get_user_cpu_time_sec();
  auto start = std::chrono::high_resolution_clock::now();
  // ==============================================================

  int iteration = 0;
  while (true) {
    iteration++;
    auto res = solver.check(active_acts);

    if (res == z3::sat) {
      std::cout << "SAT after " << (iteration - 1) << " removal step(s)" << '\n';

      z3::model model = solver.get_model();
      std::cout << "Final state values:" << '\n';
      std::cout << "  Wind Speed: " << model.eval(wind_speed) << '\n';
      std::cout << "  Power: " << model.eval(power) << '\n';
      std::cout << "  Power Need: " << model.eval(power_need) << '\n';
      std::cout << "  Park Command: " << model.eval(park_command) << '\n';
      std::cout << "  Env Signal: " << model.eval(env_signal) << '\n';
      std::cout << "  Turbine State: " << model.eval(turbine_state) << '\n';
      break;
    }

    REQUIRE(res == z3::unsat);

    z3::expr_vector core = solver.unsat_core();
    std::cout << "UNSAT core literals:" << '\n';
    for (const auto& elem: core) {
      std::cout << "  " << elem << '\n';
    }

    std::vector<int> core_idxs;
    for (const auto& elem: core) {
      auto lit_name = elem.decl().name().str();
      int idx = find_by_act_name(lit_name);
      if (idx >= 0 && gamma[idx].tag == Tag::E) {
        core_idxs.push_back(idx);
      }
    }


    if (core_idxs.empty()) {
      std::cout << "No ΓE assumptions in core — cannot proceed" << '\n';
      break;
    }

    std::sort(core_idxs.begin(), core_idxs.end(),
              [&](int a, int b) { return gamma[a].weight > gamma[b].weight; });

    int chosen = core_idxs.front();
    gamma[chosen].active = false;
    explanation.push_back({gamma[chosen].name, gamma[chosen].tag, gamma[chosen].weight});

    std::cout << "Removed: " << gamma[chosen].name
              << " [" << tag_name(gamma[chosen].tag) << "], weight=" << gamma[chosen].weight << '\n';

    active_acts = rebuild_active_vector();
  }

  std::cout << "Explanation set (removed assumptions):" << '\n';
  for (auto& e: explanation) {
    std::cout << "  " << e.name << " [" << tag_name(e.tag) << "], weight=" << e.weight << '\n';
  }

  // ==============================================================
  // Scalability Metrics: stop timer + print statistics
  auto end = std::chrono::high_resolution_clock::now();
  double cpu_end = get_user_cpu_time_sec();
  auto mem_end = get_memory_kb();
  std::chrono::duration<double> elapsed_time = end - start;

  std::cout << "=====================================================" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';

  std::cout << "Solver Statistics:" << solver.statistics() << '\n';

  auto all_vars = collect_vars_from_vector(ev_edges);
  std::cout << "Total SMT variables (syntactic): " << all_vars.size() << '\n';
  auto syntactic_vars = collect_vars_from_solver_assertions(solver);
  std::cout << "Total SMT variables in assertions: " << syntactic_vars.size() << '\n';

  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';
  std::cout << "=====================================================" << '\n';
}

TEST_CASE("14_states_wind_turbine_explainer", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "turbine" / "turbine-14-states.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("turbine-14-states.v");
  instance.write_graphviz("turbine-14-states.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);

  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 14;

  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  constexpr uint8_t NORMAL_OPERATION = 0b00000000;
  constexpr uint8_t POWER_RAMP_UP_DOWN = 0b00000001;
  constexpr uint8_t SYNC_CONNECT_DISCONNECT = 0b00000010;
  constexpr uint8_t ACCEL_DECEL = 0b00000011;
  constexpr uint8_t REDUCED_SPEED = 0b00000100;
  constexpr uint8_t BLADE_ADJUSTMENT = 0b00000101;
  constexpr uint8_t YAW_ALIGNMENT = 0b00000110;
  constexpr uint8_t ROTOR_POSITION_VERIF = 0b00000111;
  constexpr uint8_t DYNAMIC_BALANCE_CHECK = 0b00001000;
  constexpr uint8_t SUBSYSTEM_SELF_TEST = 0b00001001;
  constexpr uint8_t SENSOR_RECALIBRATION = 0b00001010;
  constexpr uint8_t POWER_SYSTEM_PRE_CHECK = 0b00001011;
  constexpr uint8_t CONTROL_LOGIC_INIT = 0b00001100;
  constexpr uint8_t SHUTDOWN = 0b00001101;
  constexpr uint8_t WIND_SPEED_THRESHOLD = 0b01100100;
  constexpr uint8_t NORMAL_POWER = 0b00011000;
  constexpr unsigned BV_WIDTH_2 = 2;
  constexpr unsigned BV_WIDTH_5 = 5;
  constexpr unsigned BV_WIDTH_8 = 8;

  auto get_signal = [&](const std::string& name) {
    return ev_edges[signal_name_to_edge_id[name]];
  };

  z3::expr wind_speed = get_signal("|D1_ComposedSystem_wind_speed_time_13|");
  z3::expr power = get_signal("|D1_ComposedSystem_power_time_13|");
  z3::expr power_need = get_signal("|D1_ComposedSystem_power_need_time_13|");
  z3::expr park_command = get_signal("|D1_ComposedSystem_park_command_time_13|");
  z3::expr env_signal = get_signal("|D1_ComposedSystem_turbine_env_time_13|");
  z3::expr turbine_state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_13|");

  z3::expr state0 = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_0|");
  solver.add(state0 == ctx.bv_val(SHUTDOWN, BV_WIDTH_8));

  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };

  enum class Tag : std::uint8_t { A,
                                  E,
                                  C };
  auto tag_name = [](Tag t) {
    switch (t) {
      case Tag::A:
        return "ΓA";
      case Tag::E:
        return "ΓE";
      case Tag::C:
        return "ΓC";
    }
    return "Γ?";
  };

  struct Assumption {
    std::string name;
    Tag tag;
    int weight;
    z3::expr act;
    z3::expr prop;
    bool active;
  };

  std::vector<Assumption> gamma;
  z3::expr_vector active_acts(ctx);

  auto add_assumption = [&](Tag tag, int weight, const std::string& label, const z3::expr& formula) {
    z3::expr act = ctx.bool_const(label.c_str());
    solver.add(z3::implies(act, formula));
    gamma.push_back({label, tag, weight, act, formula, true});
    active_acts.push_back(act);
  };

  add_assumption(Tag::A, 1, "GA_initial_shutdown", state0 == bv8(SHUTDOWN));
  add_assumption(Tag::A, 1, "GA_final_normal_operation", turbine_state == bv8(NORMAL_OPERATION));

  add_assumption(Tag::E, 2, "GE_high_wind_speed", wind_speed > bv8(WIND_SPEED_THRESHOLD));
  add_assumption(Tag::A, 1, "GA_normal_power", power == bv8(NORMAL_POWER));
  add_assumption(Tag::A, 1, "GA_power_need_2", power_need == bv8(BV_WIDTH_2));
  add_assumption(Tag::A, 1, "GA_no_park_command", park_command == bv8(NORMAL_OPERATION));

  add_assumption(Tag::C, 0, "GE_high_wind_implies_blade_adjust",
                 z3::implies(wind_speed > bv8(WIND_SPEED_THRESHOLD), env_signal == bv8(BLADE_ADJUSTMENT)));
  add_assumption(Tag::C, 0, "GE_normal_power_implies_normal_env",
                 z3::implies(power == bv8(NORMAL_POWER), env_signal == bv8(NORMAL_OPERATION)));
  add_assumption(Tag::C, 0, "GE_low_power_need_implies_reduced_speed",
                 z3::implies(power_need < bv8(BV_WIDTH_5), env_signal == bv8(REDUCED_SPEED)));
  add_assumption(Tag::E, 2, "GE_no_extreme_conditions_implies_normal_env",
                 env_signal == bv8(NORMAL_OPERATION));

  add_assumption(Tag::C, 0, "GC_env_blade_adj_implies_not_normal_op",
                 z3::implies(env_signal == bv8(BLADE_ADJUSTMENT), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_reduced_speed_implies_not_normal_op",
                 z3::implies(env_signal == bv8(REDUCED_SPEED), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_ramp_implies_not_normal_op",
                 z3::implies(env_signal == bv8(POWER_RAMP_UP_DOWN), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_yaw_implies_not_normal_op",
                 z3::implies(env_signal == bv8(YAW_ALIGNMENT), !(turbine_state == bv8(NORMAL_OPERATION))));

  add_assumption(Tag::C, 0, "GC_shutdown_implies_no_power",
                 z3::implies(turbine_state == bv8(SHUTDOWN), power == bv8(0)));
  add_assumption(Tag::C, 0, "GC_normal_op_requires_adequate_wind",
                 z3::implies(turbine_state == bv8(NORMAL_OPERATION), wind_speed > bv8(BV_WIDTH_8)));

  auto rebuild_active_vector = [&]() {
    z3::expr_vector acts(ctx);
    for (auto& a: gamma) {
      if (a.active) {
        acts.push_back(a.act);
      }
    }
    return acts;
  };

  auto find_by_act_name = [&](const std::string& n) -> int {
    for (int i = 0; i < static_cast<int>(gamma.size()); ++i) {
      if (gamma[i].act.decl().name().str() == n) {
        return i;
      }
    }
    return -1;
  };

  struct ExplItem {
    std::string name;
    Tag tag;
    int weight;
  };
  std::vector<ExplItem> explanation;

  // ==============================================================
  // Scalability Metrics: start timer
  double cpu_start = get_user_cpu_time_sec();
  auto start = std::chrono::high_resolution_clock::now();
  // ==============================================================

  int iteration = 0;
  while (true) {
    iteration++;
    auto res = solver.check(active_acts);

    if (res == z3::sat) {
      std::cout << "SAT after " << (iteration - 1) << " removal step(s)" << '\n';

      z3::model model = solver.get_model();
      std::cout << "Final state values:" << '\n';
      std::cout << "  Wind Speed: " << model.eval(wind_speed) << '\n';
      std::cout << "  Power: " << model.eval(power) << '\n';
      std::cout << "  Power Need: " << model.eval(power_need) << '\n';
      std::cout << "  Park Command: " << model.eval(park_command) << '\n';
      std::cout << "  Env Signal: " << model.eval(env_signal) << '\n';
      std::cout << "  Turbine State: " << model.eval(turbine_state) << '\n';
      constexpr unsigned intermediate_states_upper_bound = 12;
      std::cout << "Intermediate states:" << '\n';
      for (int i = 1; i <= intermediate_states_upper_bound; ++i) {
        z3::expr state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_" + std::to_string(i) + "|");
        std::cout << "  State" << i << ": " << model.eval(state) << '\n';
      }
      break;
    }

    REQUIRE(res == z3::unsat);

    z3::expr_vector core = solver.unsat_core();
    std::cout << "UNSAT core literals (iteration " << iteration << "):" << '\n';
    for (const auto& elem: core) {
      std::cout << "  " << elem << '\n';
    }

    std::vector<int> core_idxs;
    for (const auto& elem: core) {
      auto lit_name = elem.decl().name().str();
      int idx = find_by_act_name(lit_name);
      if (idx >= 0 && gamma[idx].tag == Tag::E) {
        core_idxs.push_back(idx);
      }
    }

    if (core_idxs.empty()) {
      std::cout << "No ΓE assumptions in core — structural conflict in system constraints" << '\n';
      break;
    }

    std::sort(core_idxs.begin(), core_idxs.end(),
              [&](int a, int b) { return gamma[a].weight > gamma[b].weight; });

    int chosen = core_idxs.front();
    gamma[chosen].active = false;
    explanation.push_back({gamma[chosen].name, gamma[chosen].tag, gamma[chosen].weight});

    std::cout << "Removed: " << gamma[chosen].name
              << " [" << tag_name(gamma[chosen].tag) << "], weight=" << gamma[chosen].weight << '\n';

    active_acts = rebuild_active_vector();
  }

  std::cout << "Explanation set (removed assumptions):" << '\n';
  for (auto& e: explanation) {
    std::cout << "  " << e.name << " [" << tag_name(e.tag) << "], weight=" << e.weight << '\n';
  }

  // ==============================================================
  // Scalability Metrics: stop timer + print statistics
  auto end = std::chrono::high_resolution_clock::now();
  double cpu_end = get_user_cpu_time_sec();
  auto mem_end = get_memory_kb();
  std::chrono::duration<double> elapsed_time = end - start;

  std::cout << "=====================================================" << '\n';
  std::cout << "14-State Wind Turbine Analysis" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';

  std::cout << "Solver Statistics:" << solver.statistics() << '\n';

  auto all_vars = collect_vars_from_vector(ev_edges);
  std::cout << "Total SMT variables (syntactic): " << all_vars.size() << '\n';
  auto syntactic_vars = collect_vars_from_solver_assertions(solver);
  std::cout << "Total SMT variables in assertions: " << syntactic_vars.size() << '\n';

  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';
  std::cout << "Iterations needed: " << iteration << '\n';
  std::cout << "Assumptions removed: " << explanation.size() << '\n';
  std::cout << "=====================================================" << '\n';
}

TEST_CASE("21_states_wind_turbine_explainer", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "turbine" / "turbine-21-states.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("turbine-21-states.v");
  instance.write_graphviz("turbine-21-states.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);

  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 21;

  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  constexpr uint8_t NORMAL_OPERATION = 0b00000000;
  constexpr uint8_t POWER_RAMP_UP_DOWN = 0b00000001;
  constexpr uint8_t SYNC_CONNECT_DISCONNECT = 0b00000010;
  constexpr uint8_t ACCEL_DECEL = 0b00000011;
  constexpr uint8_t REDUCED_SPEED = 0b00000100;
  constexpr uint8_t BLADE_ADJUSTMENT = 0b00000101;
  constexpr uint8_t YAW_ALIGNMENT = 0b00000110;
  constexpr uint8_t SHUTDOWN = 0b00000111;
  constexpr uint8_t SYSTEM_BOOT = 0b00001000;
  constexpr uint8_t HARDWARE_CHECK = 0b00001001;
  constexpr uint8_t FIRMWARE_LOAD = 0b00001010;
  constexpr uint8_t SELF_DIAGNOSTICS = 0b00001011;
  constexpr uint8_t ENVIRONMENTAL_CHECK = 0b00001100;
  constexpr uint8_t WIND_SPEED_EVALUATION = 0b00001101;
  constexpr uint8_t GRID_PRE_SYNC_CHECK = 0b00001110;
  constexpr uint8_t BRAKE_RELEASE = 0b00001111;
  constexpr uint8_t BEARING_WARM_UP = 0b00010000;
  constexpr uint8_t PITCH_CALIBRATION = 0b00010001;
  constexpr uint8_t INERTIA_SYNC = 0b00010010;
  constexpr uint8_t VOLTAGE_REGULATION = 0b00010011;
  constexpr uint8_t FREQUENCY_MATCHING = 0b00010100;
  constexpr uint8_t WIND_SPEED_THRESHOLD = 0b01100100;
  constexpr uint8_t NORMAL_POWER = 0b00011000;
  constexpr unsigned BV_WIDTH_2 = 2;
  constexpr unsigned BV_WIDTH_5 = 5;
  constexpr unsigned BV_WIDTH_8 = 8;
  constexpr unsigned BV_WIDTH_10 = 10;

  auto get_signal = [&](const std::string& name) {
    return ev_edges[signal_name_to_edge_id[name]];
  };

  z3::expr wind_speed = get_signal("|D1_ComposedSystem_wind_speed_time_20|");
  z3::expr power = get_signal("|D1_ComposedSystem_power_time_20|");
  z3::expr power_need = get_signal("|D1_ComposedSystem_power_need_time_20|");
  z3::expr park_command = get_signal("|D1_ComposedSystem_park_command_time_20|");
  z3::expr env_signal = get_signal("|D1_ComposedSystem_turbine_env_time_20|");
  z3::expr turbine_state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_20|");

  z3::expr state0 = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_0|");
  solver.add(state0 == ctx.bv_val(SHUTDOWN, BV_WIDTH_8));

  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };

  enum class Tag : std::uint8_t { A,
                                  E,
                                  C };
  auto tag_name = [](Tag t) {
    switch (t) {
      case Tag::A:
        return "ΓA";
      case Tag::E:
        return "ΓE";
      case Tag::C:
        return "ΓC";
    }
    return "Γ?";
  };

  struct Assumption {
    std::string name;
    Tag tag;
    int weight;
    z3::expr act;
    z3::expr prop;
    bool active;
  };

  std::vector<Assumption> gamma;
  z3::expr_vector active_acts(ctx);

  auto add_assumption = [&](Tag tag, int weight, const std::string& label, const z3::expr& formula) {
    z3::expr act = ctx.bool_const(label.c_str());
    solver.add(z3::implies(act, formula));
    gamma.push_back({label, tag, weight, act, formula, true});
    active_acts.push_back(act);
  };

  add_assumption(Tag::A, 1, "GA_initial_shutdown", state0 == bv8(SHUTDOWN));
  add_assumption(Tag::A, 1, "GA_final_normal_operation", turbine_state == bv8(NORMAL_OPERATION));

  add_assumption(Tag::E, 2, "GE_high_wind_speed", wind_speed > bv8(WIND_SPEED_THRESHOLD));
  add_assumption(Tag::A, 1, "GA_normal_power", power == bv8(NORMAL_POWER));
  add_assumption(Tag::A, 1, "GA_power_need_2", power_need == bv8(BV_WIDTH_2));
  add_assumption(Tag::A, 1, "GA_no_park_command", park_command == bv8(NORMAL_OPERATION));

  add_assumption(Tag::C, 0, "GC_high_wind_implies_blade_adjust",
                 z3::implies(wind_speed > bv8(WIND_SPEED_THRESHOLD), env_signal == bv8(BLADE_ADJUSTMENT)));
  add_assumption(Tag::C, 0, "GC_normal_power_implies_normal_env",
                 z3::implies(power == bv8(NORMAL_POWER), env_signal == bv8(NORMAL_OPERATION)));
  add_assumption(Tag::C, 0, "GC_low_power_implies_reduced_speed",
                 z3::implies(power > bv8(0) && power <= bv8(BV_WIDTH_10), env_signal == bv8(REDUCED_SPEED)));

  add_assumption(Tag::C, 0, "GC_no_extreme_conditions_implies_normal_env",
                 env_signal == bv8(NORMAL_OPERATION));

  add_assumption(Tag::C, 0, "GC_env_blade_adj_implies_not_normal_op",
                 z3::implies(env_signal == bv8(BLADE_ADJUSTMENT), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_reduced_speed_implies_not_normal_op",
                 z3::implies(env_signal == bv8(REDUCED_SPEED), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_ramp_implies_not_normal_op",
                 z3::implies(env_signal == bv8(POWER_RAMP_UP_DOWN), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_yaw_implies_not_normal_op",
                 z3::implies(env_signal == bv8(YAW_ALIGNMENT), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_voltage_implies_not_normal_op",
                 z3::implies(env_signal == bv8(VOLTAGE_REGULATION), !(turbine_state == bv8(NORMAL_OPERATION))));
  add_assumption(Tag::C, 0, "GC_env_frequency_implies_not_normal_op",
                 z3::implies(env_signal == bv8(FREQUENCY_MATCHING), !(turbine_state == bv8(NORMAL_OPERATION))));

  add_assumption(Tag::C, 0, "GC_shutdown_implies_no_power",
                 z3::implies(turbine_state == bv8(SHUTDOWN), power == bv8(0)));
  add_assumption(Tag::C, 0, "GC_normal_op_requires_adequate_wind",
                 z3::implies(turbine_state == bv8(NORMAL_OPERATION), wind_speed > bv8(BV_WIDTH_5)));
  add_assumption(Tag::C, 0, "GC_boot_implies_low_power",
                 z3::implies(turbine_state == bv8(SYSTEM_BOOT), power < bv8(BV_WIDTH_5)));
  add_assumption(Tag::C, 0, "GC_hardware_check_early_state",
                 z3::implies(turbine_state == bv8(HARDWARE_CHECK), state0 == bv8(SYSTEM_BOOT)));

  auto rebuild_active_vector = [&]() {
    z3::expr_vector acts(ctx);
    for (auto& a: gamma) {
      if (a.active) {
        acts.push_back(a.act);
      }
    }
    return acts;
  };

  auto find_by_act_name = [&](const std::string& n) -> int {
    for (int i = 0; i < static_cast<int>(gamma.size()); ++i) {
      if (gamma[i].act.decl().name().str() == n) {
        return i;
      }
    }
    return -1;
  };

  struct ExplItem {
    std::string name;
    Tag tag;
    int weight;
  };
  std::vector<ExplItem> explanation;

  // ==============================================================
  // Scalability Metrics: start timer
  double cpu_start = get_user_cpu_time_sec();
  auto start = std::chrono::high_resolution_clock::now();
  // ==============================================================

  int iteration = 0;
  while (true) {
    iteration++;
    auto res = solver.check(active_acts);

    if (res == z3::sat) {
      std::cout << "SAT after " << (iteration - 1) << " removal step(s)" << '\n';

      z3::model model = solver.get_model();
      std::cout << "Final state values:" << '\n';
      std::cout << "  Wind Speed: " << model.eval(wind_speed) << '\n';
      std::cout << "  Power: " << model.eval(power) << '\n';
      std::cout << "  Power Need: " << model.eval(power_need) << '\n';
      std::cout << "  Park Command: " << model.eval(park_command) << '\n';
      std::cout << "  Env Signal: " << model.eval(env_signal) << '\n';
      std::cout << "  Turbine State: " << model.eval(turbine_state) << '\n';
      constexpr unsigned intermediate_states_5 = 5;
      constexpr unsigned intermediate_states_15 = 15;
      constexpr unsigned intermediate_states_19 = 19;
      std::cout << "Key intermediate states:" << '\n';
      for (int i = 1; i <= intermediate_states_5; ++i) {
        z3::expr state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_" + std::to_string(i) + "|");
        std::cout << "  State" << i << ": " << model.eval(state) << '\n';
      }
      std::cout << "  ..." << '\n';
      for (int i = intermediate_states_15; i <= intermediate_states_19; ++i) {
        z3::expr state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_" + std::to_string(i) + "|");
        std::cout << "  State" << i << ": " << model.eval(state) << '\n';
      }
      break;
    }

    REQUIRE(res == z3::unsat);

    z3::expr_vector core = solver.unsat_core();
    std::cout << "UNSAT core literals (iteration " << iteration << "):" << '\n';
    for (const auto& elem: core) {
      std::cout << "  " << elem << '\n';
    }

    std::vector<int> core_idxs;
    for (const auto& elem: core) {
      auto lit_name = elem.decl().name().str();
      int idx = find_by_act_name(lit_name);
      if (idx >= 0 && gamma[idx].tag == Tag::E) {
        core_idxs.push_back(idx);
      }
    }


    if (core_idxs.empty()) {
      std::cout << "No ΓE assumptions in core — structural conflict in system constraints" << '\n';
      std::cout << "This indicates the requirements (ΓA) or system constraints (ΓC) are inconsistent" << '\n';
      break;
    }

    std::sort(core_idxs.begin(), core_idxs.end(),
              [&](int a, int b) { return gamma[a].weight > gamma[b].weight; });

    int chosen = core_idxs.front();
    gamma[chosen].active = false;
    explanation.push_back({gamma[chosen].name, gamma[chosen].tag, gamma[chosen].weight});

    std::cout << "Removed: " << gamma[chosen].name
              << " [" << tag_name(gamma[chosen].tag) << "], weight=" << gamma[chosen].weight << '\n';

    active_acts = rebuild_active_vector();
  }

  std::cout << "Explanation set (removed assumptions):" << '\n';
  for (auto& e: explanation) {
    std::cout << "  " << e.name << " [" << tag_name(e.tag) << "], weight=" << e.weight << '\n';
  }

  // ==============================================================
  // Scalability Metrics: stop timer + print statistics
  auto end = std::chrono::high_resolution_clock::now();
  double cpu_end = get_user_cpu_time_sec();
  auto mem_end = get_memory_kb();
  std::chrono::duration<double> elapsed_time = end - start;

  std::cout << "=====================================================" << '\n';
  std::cout << "21-State Wind Turbine Analysis" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';

  std::cout << "Solver Statistics:" << solver.statistics() << '\n';

  auto all_vars = collect_vars_from_vector(ev_edges);
  std::cout << "Total SMT variables (syntactic): " << all_vars.size() << '\n';
  auto syntactic_vars = collect_vars_from_solver_assertions(solver);
  std::cout << "Total SMT variables in assertions: " << syntactic_vars.size() << '\n';

  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';
  std::cout << "Iterations needed: " << iteration << '\n';
  std::cout << "Assumptions removed: " << explanation.size() << '\n';
  std::cout << "Remaining active assumptions: " << active_acts.size() << '\n';

  std::cout << "State space complexity analysis:" << '\n';
  std::cout << "  Total states: 21" << '\n';
  std::cout << "  Environmental signals: " << gamma.size() << '\n';
  std::cout << "  System constraints: " << std::count_if(gamma.begin(), gamma.end(), [](const Assumption& a) { return a.tag == Tag::C; }) << '\n';
  std::cout << "=====================================================" << '\n';
}

TEST_CASE("28_states_wind_turbine_explainer", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "turbine" / "28_states_wind_turbine.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("28_states_wind_turbine.v");
  instance.write_graphviz("28_states_wind_turbine.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);

  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 28;

  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  constexpr uint8_t NORMAL_OPERATION = 0b00000000;
  constexpr uint8_t POWER_RAMP_UP_DOWN = 0b00000001;
  constexpr uint8_t SYNC_CONNECT_DISCONNECT = 0b00000010;
  constexpr uint8_t ACCEL_DECEL = 0b00000011;
  constexpr uint8_t REDUCED_SPEED = 0b00000100;
  constexpr uint8_t BLADE_ADJUSTMENT = 0b00000101;
  constexpr uint8_t YAW_ALIGNMENT = 0b00000110;
  constexpr uint8_t ROTOR_POSITION_VERIF = 0b00000111;
  constexpr uint8_t DYNAMIC_BALANCE_CHECK = 0b00001000;
  constexpr uint8_t SUBSYSTEM_SELF_TEST = 0b00001001;
  constexpr uint8_t SENSOR_RECALIBRATION = 0b00001010;
  constexpr uint8_t POWER_SYSTEM_PRE_CHECK = 0b00001011;
  constexpr uint8_t CONTROL_LOGIC_INIT = 0b00001100;
  constexpr uint8_t SAFETY_SYSTEMS_ENGAGE = 0b00001101;
  constexpr uint8_t SYSTEM_BOOT = 0b00001110;
  constexpr uint8_t HARDWARE_CHECK = 0b00001111;
  constexpr uint8_t FIRMWARE_LOAD = 0b00010000;
  constexpr uint8_t SELF_DIAGNOSTICS = 0b00010001;
  constexpr uint8_t ENVIRONMENTAL_CHECK = 0b00010010;
  constexpr uint8_t WIND_SPEED_EVALUATION = 0b00010011;
  constexpr uint8_t GRID_PRE_SYNC_CHECK = 0b00010100;
  constexpr uint8_t BRAKE_RELEASE = 0b00010101;
  constexpr uint8_t BEARING_WARM_UP = 0b00010110;
  constexpr uint8_t PITCH_CALIBRATION = 0b00010111;
  constexpr uint8_t INERTIA_SYNC = 0b00011000;
  constexpr uint8_t VOLTAGE_REGULATION = 0b00011001;
  constexpr uint8_t FREQUENCY_MATCHING = 0b00011010;
  constexpr uint8_t SHUTDOWN = 0b00011011;
  constexpr uint8_t WIND_SPEED_THRESHOLD = 0b01100100;
  constexpr uint8_t NORMAL_POWER = 0b00011000;
  constexpr unsigned BV_WIDTH_2 = 2;
  constexpr unsigned BV_WIDTH_5 = 5;
  constexpr unsigned BV_WIDTH_8 = 8;
  constexpr unsigned BV_WIDTH_10 = 10;
  auto get_signal = [&](const std::string& name) {
    return ev_edges[signal_name_to_edge_id[name]];
  };

  z3::expr wind_speed = get_signal("|D1_ComposedSystem_wind_speed_time_27|");
  z3::expr power = get_signal("|D1_ComposedSystem_power_time_27|");
  z3::expr power_need = get_signal("|D1_ComposedSystem_power_need_time_27|");
  z3::expr park_command = get_signal("|D1_ComposedSystem_park_command_time_27|");
  z3::expr env_signal = get_signal("|D1_ComposedSystem_turbine_env_time_27|");
  z3::expr turbine_state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_27|");

  z3::expr state0 = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_0|");
  solver.add(state0 == ctx.bv_val(SHUTDOWN, BV_WIDTH_8));

  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };

  enum class Tag : std::uint8_t { A,
                                  E,
                                  C };
  auto tag_name = [](Tag t) {
    switch (t) {
      case Tag::A:
        return "ΓA";
      case Tag::E:
        return "ΓE";
      case Tag::C:
        return "ΓC";
    }
    return "Γ?";
  };

  struct Assumption {
    std::string name;
    Tag tag;
    int weight;
    z3::expr act;
    z3::expr prop;
    bool active;
  };

  std::vector<Assumption> gamma;
  z3::expr_vector active_acts(ctx);

  auto add_assumption = [&](Tag tag, int weight, const std::string& label, const z3::expr& formula) {
    z3::expr act = ctx.bool_const(label.c_str());
    solver.add(z3::implies(act, formula));
    gamma.push_back({label, tag, weight, act, formula, true});
    active_acts.push_back(act);
  };

  add_assumption(Tag::A, 1, "GA_initial_shutdown", state0 == bv8(SHUTDOWN));
  add_assumption(Tag::A, 1, "GA_final_normal_operation", turbine_state == bv8(NORMAL_OPERATION));

  add_assumption(Tag::E, 2, "GE_high_wind_speed", wind_speed > bv8(WIND_SPEED_THRESHOLD));
  add_assumption(Tag::A, 1, "GA_normal_power", power == bv8(NORMAL_POWER));
  add_assumption(Tag::A, 1, "GA_power_need_2", power_need == bv8(BV_WIDTH_2));
  add_assumption(Tag::A, 1, "GA_no_park_command", park_command == bv8(NORMAL_OPERATION));

  add_assumption(Tag::C, 0, "GC_high_wind_implies_blade_adjust",
                 z3::implies(wind_speed > bv8(WIND_SPEED_THRESHOLD), env_signal == bv8(BLADE_ADJUSTMENT)));
  add_assumption(Tag::C, 0, "GC_normal_power_implies_normal_env",
                 z3::implies(power == bv8(NORMAL_POWER), env_signal == bv8(NORMAL_OPERATION)));
  add_assumption(Tag::C, 0, "GC_low_power_implies_reduced_speed",
                 z3::implies(power > bv8(0) && power <= bv8(BV_WIDTH_10), env_signal == bv8(REDUCED_SPEED)));

  add_assumption(Tag::C, 0, "GC_no_extreme_conditions_implies_normal_env",
                 env_signal == bv8(NORMAL_OPERATION));
  add_assumption(Tag::E, 2, "GE_dynamic_balance_conditions",
                 env_signal != bv8(DYNAMIC_BALANCE_CHECK));
  add_assumption(Tag::E, 2, "GE_subsystem_self_test_conditions",
                 env_signal != bv8(SUBSYSTEM_SELF_TEST));
  add_assumption(Tag::E, 2, "GE_sensor_recalibration_conditions",
                 env_signal != bv8(SENSOR_RECALIBRATION));

  add_assumption(Tag::E, 1, "GE_bearing_warmup_conditions",
                 env_signal != bv8(BEARING_WARM_UP));
  add_assumption(Tag::E, 1, "GE_pitch_calibration_conditions",
                 env_signal != bv8(PITCH_CALIBRATION));
  add_assumption(Tag::E, 1, "GE_inertia_sync_conditions",
                 env_signal != bv8(INERTIA_SYNC));

  std::vector<uint8_t> non_normal_env_states = {
      BLADE_ADJUSTMENT, REDUCED_SPEED, POWER_RAMP_UP_DOWN, YAW_ALIGNMENT,
      VOLTAGE_REGULATION, FREQUENCY_MATCHING, ROTOR_POSITION_VERIF,
      DYNAMIC_BALANCE_CHECK, SUBSYSTEM_SELF_TEST, SENSOR_RECALIBRATION,
      BEARING_WARM_UP, PITCH_CALIBRATION, INERTIA_SYNC};

  for (uint8_t state: non_normal_env_states) {
    add_assumption(Tag::C, 0, "GC_env_" + std::to_string(state) + "_implies_not_normal_op",
                   z3::implies(env_signal == bv8(state), !(turbine_state == bv8(NORMAL_OPERATION))));
  }

  add_assumption(Tag::C, 0, "GC_shutdown_implies_no_power",
                 z3::implies(turbine_state == bv8(SHUTDOWN), power == bv8(0)));
  add_assumption(Tag::C, 0, "GC_normal_op_requires_adequate_wind",
                 z3::implies(turbine_state == bv8(NORMAL_OPERATION), wind_speed > bv8(BV_WIDTH_5)));
  add_assumption(Tag::C, 0, "GC_boot_implies_low_power",
                 z3::implies(turbine_state == bv8(SYSTEM_BOOT), power < bv8(BV_WIDTH_5)));
  add_assumption(Tag::C, 0, "GC_hardware_check_early_state",
                 z3::implies(turbine_state == bv8(HARDWARE_CHECK), state0 == bv8(SYSTEM_BOOT)));
  add_assumption(Tag::C, 0, "GC_safety_systems_require_power_check",
                 z3::implies(turbine_state == bv8(SAFETY_SYSTEMS_ENGAGE), power_need > bv8(0)));
  add_assumption(Tag::C, 0, "GC_self_diagnostics_early_phase",
                 z3::implies(turbine_state == bv8(SELF_DIAGNOSTICS), state0 == bv8(HARDWARE_CHECK)));

  auto rebuild_active_vector = [&]() {
    z3::expr_vector acts(ctx);
    for (auto& a: gamma) {
      if (a.active) {
        acts.push_back(a.act);
      }
    }
    return acts;
  };

  auto find_by_act_name = [&](const std::string& n) -> int {
    for (int i = 0; i < static_cast<int>(gamma.size()); ++i) {
      if (gamma[i].act.decl().name().str() == n) {
        return i;
      }
    }
    return -1;
  };

  struct ExplItem {
    std::string name;
    Tag tag;
    int weight;
  };
  std::vector<ExplItem> explanation;

  // ==============================================================
  // Scalability Metrics: start timer
  double cpu_start = get_user_cpu_time_sec();
  auto start = std::chrono::high_resolution_clock::now();
  // ==============================================================

  int iteration = 0;
  size_t max_iterations = gamma.size();
  while (iteration < max_iterations) {
    iteration++;
    auto res = solver.check(active_acts);

    if (res == z3::sat) {
      std::cout << "SAT after " << (iteration - 1) << " removal step(s)" << '\n';
      constexpr unsigned intermediate_states_10 = 10;
      constexpr unsigned intermediate_states_12 = 12;
      constexpr unsigned intermediate_states_22 = 22;
      constexpr unsigned intermediate_states_26 = 26;
      z3::model model = solver.get_model();
      std::cout << "Final state values:" << '\n';
      std::cout << "  Wind Speed: " << model.eval(wind_speed) << '\n';
      std::cout << "  Power: " << model.eval(power) << '\n';
      std::cout << "  Power Need: " << model.eval(power_need) << '\n';
      std::cout << "  Park Command: " << model.eval(park_command) << '\n';
      std::cout << "  Env Signal: " << model.eval(env_signal) << '\n';
      std::cout << "  Turbine State: " << model.eval(turbine_state) << '\n';

      std::cout << "Key intermediate states:" << '\n';
      // Early boot sequence
      for (int i = 1; i <= 3; ++i) {
        z3::expr state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_" + std::to_string(i) + "|");
        std::cout << "  State" << i << ": " << model.eval(state) << '\n';
      }
      for (int i = intermediate_states_10; i <= intermediate_states_12; ++i) {
        z3::expr state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_" + std::to_string(i) + "|");
        std::cout << "  State" << i << ": " << model.eval(state) << '\n';
      }
      for (int i = intermediate_states_22; i <= intermediate_states_26; ++i) {
        z3::expr state = get_signal("|D1_ComposedSystem_turbine_turbine_state_time_" + std::to_string(i) + "|");
        std::cout << "  State" << i << ": " << model.eval(state) << '\n';
      }
      break;
    }

    REQUIRE(res == z3::unsat);

    z3::expr_vector core = solver.unsat_core();
    std::cout << "UNSAT core literals (iteration " << iteration << "):" << '\n';
    for (const auto& elem: core) {
      std::cout << "  " << elem << '\n';
    }

    std::vector<int> core_idxs;
    for (const auto& elem: core) {
      auto lit_name = elem.decl().name().str();
      int idx = find_by_act_name(lit_name);
      if (idx >= 0 && gamma[idx].tag == Tag::E) {
        core_idxs.push_back(idx);
      }
    }

    if (core_idxs.empty()) {
      std::cout << "No ΓE assumptions in core — structural conflict in system constraints" << '\n';
      std::cout << "This indicates the requirements (ΓA) or system constraints (ΓC) are inconsistent" << '\n';
      break;
    }

    std::sort(core_idxs.begin(), core_idxs.end(),
              [&](int a, int b) { return gamma[a].weight > gamma[b].weight; });

    int chosen = core_idxs.front();
    gamma[chosen].active = false;
    explanation.push_back({gamma[chosen].name, gamma[chosen].tag, gamma[chosen].weight});

    std::cout << "Removed: " << gamma[chosen].name
              << " [" << tag_name(gamma[chosen].tag) << "], weight=" << gamma[chosen].weight << '\n';

    active_acts = rebuild_active_vector();
  }

  if (iteration >= max_iterations) {
    std::cout << "Reached maximum iterations without finding SAT solution" << '\n';
  }

  std::cout << "Explanation set (removed assumptions):" << '\n';
  for (auto& e: explanation) {
    std::cout << "  " << e.name << " [" << tag_name(e.tag) << "], weight=" << e.weight << '\n';
  }

  // ==============================================================
  // Scalability Metrics: stop timer + print statistics
  auto end = std::chrono::high_resolution_clock::now();
  double cpu_end = get_user_cpu_time_sec();
  auto mem_end = get_memory_kb();
  std::chrono::duration<double> elapsed_time = end - start;

  std::cout << "=====================================================" << '\n';
  std::cout << "28-State Wind Turbine Analysis" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';

  std::cout << "Solver Statistics:" << solver.statistics() << '\n';

  auto all_vars = collect_vars_from_vector(ev_edges);
  std::cout << "Total SMT variables (syntactic): " << all_vars.size() << '\n';
  auto syntactic_vars = collect_vars_from_solver_assertions(solver);
  std::cout << "Total SMT variables in assertions: " << syntactic_vars.size() << '\n';

  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';
  std::cout << "Iterations needed: " << iteration << '\n';
  std::cout << "Assumptions removed: " << explanation.size() << '\n';
  std::cout << "Remaining active assumptions: " << active_acts.size() << '\n';

  auto gammaA_count = std::count_if(gamma.begin(), gamma.end(),
                                    [](const Assumption& a) { return a.tag == Tag::A && a.active; });
  auto gammaE_count = std::count_if(gamma.begin(), gamma.end(),
                                    [](const Assumption& a) { return a.tag == Tag::E && a.active; });
  auto gammaC_count = std::count_if(gamma.begin(), gamma.end(),
                                    [](const Assumption& a) { return a.tag == Tag::C && a.active; });

  std::cout << "State space complexity analysis:" << '\n';
  std::cout << "  Total states: 28" << '\n';
  std::cout << "  Active ΓA (requirements): " << gammaA_count << '\n';
  std::cout << "  Active ΓE (environmental): " << gammaE_count << '\n';
  std::cout << "  Active ΓC (constraints): " << gammaC_count << '\n';
  std::cout << "  Total active assumptions: " << (gammaA_count + gammaE_count + gammaC_count) << '\n';
  std::cout << "=====================================================" << '\n';
}