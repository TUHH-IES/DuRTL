//
// Created by moham on 11/13/2025.
//

#include <common_definitions.hpp>
#include <ducode/ExpLib/Analysis.h>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>

#include <catch2/catch_test_macros.hpp>
#include <z3++.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

TEST_CASE("traffic_resolve", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "traffic" / "traffic.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("traffic.v");
  instance.write_graphviz("traffic.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);

  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 11;
  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  constexpr uint8_t PROCEED = 0b00000000;
  constexpr uint8_t SLOW = 0b00000001;
  constexpr uint8_t STOP = 0b00000010;
  constexpr uint8_t IDLE = 0b00010000;
  constexpr uint8_t ALIGN = 0b00010001;
  constexpr uint8_t GAP = 0b00010010;
  constexpr uint8_t ACCEL = 0b00010011;
  constexpr uint8_t SYNC = 0b00010100;
  constexpr uint8_t CRUISE = 0b00010101;
  constexpr uint8_t CRAWL = 0b00010110;
  constexpr uint8_t HOLD = 0b00010111;
  constexpr uint8_t BRAKE = 0b00011000;
  constexpr unsigned BV_WIDTH_8 = 8;


  auto get_signal = [&](const std::string& name) { return ev_edges[signal_name_to_edge_id[name]]; };

  z3::expr battery_ok = get_signal("|D1_ComposedSystem_battery_ok_time_10|");
  z3::expr friction_ok = get_signal("|D1_ComposedSystem_friction_ok_time_10|");
  z3::expr occ = get_signal("|D1_ComposedSystem_occ_time_10|");
  z3::expr ped = get_signal("|D1_ComposedSystem_ped_time_10|");
  z3::expr vehicle_state = get_signal("|D1_ComposedSystem_vehicle_state_time_10|");
  z3::expr traffic_command = get_signal("|D1_ComposedSystem_traffic_command_time_10|");

  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };
  auto is = [&](const z3::expr& s, uint8_t v) { return s == bv8(v); };

  z3::expr ProceedOut = is(vehicle_state, ALIGN) || is(vehicle_state, GAP) ||
                        is(vehicle_state, ACCEL) || is(vehicle_state, SYNC) ||
                        is(vehicle_state, CRUISE);
  z3::expr SlowOut = is(vehicle_state, CRAWL);
  z3::expr StopOut = is(vehicle_state, BRAKE) || is(vehicle_state, HOLD) || is(vehicle_state, IDLE);

  solver.add(z3::implies(ProceedOut, !SlowOut && !StopOut));
  solver.add(z3::implies(SlowOut, !ProceedOut && !StopOut));
  solver.add(z3::implies(StopOut, !ProceedOut && !SlowOut));

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

  add_assumption(Tag::A, 1, "GA_cmd_PROCEED", traffic_command == bv8(PROCEED));

  add_assumption(Tag::E, 2, "GE_ped_present", ped == ctx.bv_val(1, 1));
  add_assumption(Tag::A, 2, "GE_occ_clear", occ == ctx.bv_val(0, 1));
  add_assumption(Tag::A, 2, "GE_friction_ok", friction_ok == ctx.bv_val(1, 1));
  add_assumption(Tag::A, 2, "GE_battery_ok", battery_ok == ctx.bv_val(1, 1));

  add_assumption(Tag::C, 0, "GC_cmd_PROCEED_implies_ProceedOut",
                 z3::implies(traffic_command == bv8(PROCEED), ProceedOut));
  add_assumption(Tag::C, 0, "GC_cmd_Stop_implies_StopOut",
                 z3::implies(traffic_command == bv8(PROCEED), ProceedOut));
  add_assumption(Tag::C, 0, "GE_ped_implies_StopOut",
                 z3::implies(ped == ctx.bv_val(1, 1), StopOut));

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

  std::cout << "=====================================================" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';
  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';

  // ==============================================================
}


TEST_CASE("traffic_two_vehicles_resolver", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "traffic" / "extraffic.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("extraffic.v");
  instance.write_graphviz("extraffic.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 11;
  constexpr unsigned BV_WIDTH_8 = 8;
  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  auto get = [&](const std::string& name) { return ev_edges[signal_name_to_edge_id.at(name)]; };
  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };
  auto is = [&](const z3::expr& s, uint8_t v) { return s == bv8(v); };

  constexpr uint8_t PROCEED = 0b00000000;
  constexpr uint8_t SLOW = 0b00000001;
  constexpr uint8_t STOP = 0b00000010;
  constexpr uint8_t IDLE = 0b00010000;
  constexpr uint8_t ALIGN = 0b00010001;
  constexpr uint8_t GAP = 0b00010010;
  constexpr uint8_t ACCEL = 0b00010011;
  constexpr uint8_t SYNC = 0b00010100;
  constexpr uint8_t CRUISE = 0b00010101;
  constexpr uint8_t CRAWL = 0b00010110;
  constexpr uint8_t HOLD = 0b00010111;
  constexpr uint8_t BRAKE = 0b00011000;

  z3::expr battery_ok1 = get("|D1_ComposedSystem_battery_ok1_time_10|");
  z3::expr friction_ok1 = get("|D1_ComposedSystem_friction_ok1_time_10|");
  z3::expr occ1 = get("|D1_ComposedSystem_occ1_time_10|");
  z3::expr ped1 = get("|D1_ComposedSystem_ped1_time_10|");
  z3::expr vehicle1_state = get("|D1_ComposedSystem_vehicle1_state_time_10|");

  z3::expr battery_ok2 = get("|D1_ComposedSystem_battery_ok2_time_10|");
  z3::expr friction_ok2 = get("|D1_ComposedSystem_friction_ok2_time_10|");
  z3::expr occ2 = get("|D1_ComposedSystem_occ2_time_10|");
  z3::expr ped2 = get("|D1_ComposedSystem_ped2_time_10|");
  z3::expr vehicle2_state = get("|D1_ComposedSystem_vehicle2_state_time_10|");

  z3::expr traffic_command = get("|D1_ComposedSystem_traffic_command_time_10|");

  z3::expr ProceedOut1 = is(vehicle1_state, ALIGN) || is(vehicle1_state, GAP) ||
                         is(vehicle1_state, ACCEL) || is(vehicle1_state, SYNC) ||
                         is(vehicle1_state, CRUISE);
  z3::expr SlowOut1 = is(vehicle1_state, CRAWL);
  z3::expr StopOut1 = is(vehicle1_state, BRAKE) || is(vehicle1_state, HOLD) || is(vehicle1_state, IDLE);

  z3::expr ProceedOut2 = is(vehicle2_state, ALIGN) || is(vehicle2_state, GAP) ||
                         is(vehicle2_state, ACCEL) || is(vehicle2_state, SYNC) ||
                         is(vehicle2_state, CRUISE);
  z3::expr SlowOut2 = is(vehicle2_state, CRAWL);
  z3::expr StopOut2 = is(vehicle2_state, BRAKE) || is(vehicle2_state, HOLD) || is(vehicle2_state, IDLE);

  solver.add(z3::implies(ProceedOut1, !SlowOut1 && !StopOut1));
  solver.add(z3::implies(SlowOut1, !ProceedOut1 && !StopOut1));
  solver.add(z3::implies(StopOut1, !ProceedOut1 && !SlowOut1));
  solver.add(z3::implies(ProceedOut2, !SlowOut2 && !StopOut2));
  solver.add(z3::implies(SlowOut2, !ProceedOut2 && !StopOut2));
  solver.add(z3::implies(StopOut2, !ProceedOut2 && !SlowOut2));

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

  add_assumption(Tag::E, 2, "GE_ped1_present", ped1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped2_clear", ped2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ1_clear", occ1 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ2_clear", occ2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_fric1_ok", friction_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric2_ok", friction_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt1_ok", battery_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt2_ok", battery_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped1_implies_StopOut1", implies(ped1 == ctx.bv_val(1, 1), StopOut1));
  add_assumption(Tag::E, 2, "GE_ped2_implies_StopOut2", implies(ped2 == ctx.bv_val(1, 1), StopOut2));
  add_assumption(Tag::E, 2, "GE_StopOut1_implies_occ1", implies(StopOut1, occ1 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut2_implies_occ2", implies(StopOut2, occ2 == ctx.bv_val(1, 1)));
  // ΓA: addressee assumptions (locked)
  add_assumption(Tag::A, 1, "GA_cmd_PROCEED", traffic_command == bv8(PROCEED));
  //solver.add(traffic_command == bv8(PROCEED)); // instead of GA_cmd_PROCEED with act literal

  // ΓC: connectivity assumptions (locked)
  add_assumption(Tag::C, 0, "GC_cmd_PROCEED_implies_ProceedOut1", implies(traffic_command == bv8(PROCEED), ProceedOut1));
  add_assumption(Tag::C, 0, "GC_ProceedOut1_implies_ProceedOut2", implies(ProceedOut1, ProceedOut2));
  add_assumption(Tag::C, 0, "GC_cmd_Stop_implies_StopOut1", implies(traffic_command == bv8(STOP), StopOut1));
  add_assumption(Tag::C, 0, "GC_StopOut1_implies_StopOut2", implies(StopOut1, StopOut2));

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

  std::cout << "=====================================================" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';
  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';

  // ==============================================================
}

TEST_CASE("traffic_three_vehicles_resolver", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "traffic" / "extraffic3.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("extraffic3.v");
  instance.write_graphviz("extraffic3.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 11;
  constexpr unsigned BV_WIDTH_8 = 8;
  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  auto get = [&](const std::string& name) { return ev_edges[signal_name_to_edge_id.at(name)]; };
  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };
  auto is = [&](const z3::expr& s, uint8_t v) { return s == bv8(v); };

  constexpr uint8_t PROCEED = 0b00000000;
  constexpr uint8_t SLOW = 0b00000001;
  constexpr uint8_t STOP = 0b00000010;
  constexpr uint8_t IDLE = 0b00010000;
  constexpr uint8_t ALIGN = 0b00010001;
  constexpr uint8_t GAP = 0b00010010;
  constexpr uint8_t ACCEL = 0b00010011;
  constexpr uint8_t SYNC = 0b00010100;
  constexpr uint8_t CRUISE = 0b00010101;
  constexpr uint8_t CRAWL = 0b00010110;
  constexpr uint8_t HOLD = 0b00010111;
  constexpr uint8_t BRAKE = 0b00011000;

  z3::expr battery_ok1 = get("|D1_ComposedSystem_battery_ok1_time_10|");
  z3::expr friction_ok1 = get("|D1_ComposedSystem_friction_ok1_time_10|");
  z3::expr occ1 = get("|D1_ComposedSystem_occ1_time_10|");
  z3::expr ped1 = get("|D1_ComposedSystem_ped1_time_10|");
  z3::expr vehicle1_state = get("|D1_ComposedSystem_vehicle1_state_time_10|");

  z3::expr battery_ok2 = get("|D1_ComposedSystem_battery_ok2_time_10|");
  z3::expr friction_ok2 = get("|D1_ComposedSystem_friction_ok2_time_10|");
  z3::expr occ2 = get("|D1_ComposedSystem_occ2_time_10|");
  z3::expr ped2 = get("|D1_ComposedSystem_ped2_time_10|");
  z3::expr vehicle2_state = get("|D1_ComposedSystem_vehicle2_state_time_10|");

  z3::expr battery_ok3 = get("|D1_ComposedSystem_battery_ok3_time_10|");
  z3::expr friction_ok3 = get("|D1_ComposedSystem_friction_ok3_time_10|");
  z3::expr occ3 = get("|D1_ComposedSystem_occ3_time_10|");
  z3::expr ped3 = get("|D1_ComposedSystem_ped3_time_10|");
  z3::expr vehicle3_state = get("|D1_ComposedSystem_vehicle3_state_time_10|");

  z3::expr traffic_command = get("|D1_ComposedSystem_traffic_command_time_10|");

  z3::expr ProceedOut1 = is(vehicle1_state, ALIGN) || is(vehicle1_state, GAP) ||
                         is(vehicle1_state, ACCEL) || is(vehicle1_state, SYNC) ||
                         is(vehicle1_state, CRUISE);
  z3::expr SlowOut1 = is(vehicle1_state, CRAWL);
  z3::expr StopOut1 = is(vehicle1_state, BRAKE) || is(vehicle1_state, HOLD) || is(vehicle1_state, IDLE);

  z3::expr ProceedOut2 = is(vehicle2_state, ALIGN) || is(vehicle2_state, GAP) ||
                         is(vehicle2_state, ACCEL) || is(vehicle2_state, SYNC) ||
                         is(vehicle2_state, CRUISE);
  z3::expr SlowOut2 = is(vehicle2_state, CRAWL);
  z3::expr StopOut2 = is(vehicle2_state, BRAKE) || is(vehicle2_state, HOLD) || is(vehicle2_state, IDLE);

  z3::expr ProceedOut3 = is(vehicle3_state, ALIGN) || is(vehicle3_state, GAP) ||
                         is(vehicle3_state, ACCEL) || is(vehicle3_state, SYNC) ||
                         is(vehicle3_state, CRUISE);
  z3::expr SlowOut3 = is(vehicle3_state, CRAWL);
  z3::expr StopOut3 = is(vehicle3_state, BRAKE) || is(vehicle3_state, HOLD) || is(vehicle3_state, IDLE);

  solver.add(z3::implies(ProceedOut1, !SlowOut1 && !StopOut1));
  solver.add(z3::implies(SlowOut1, !ProceedOut1 && !StopOut1));
  solver.add(z3::implies(StopOut1, !ProceedOut1 && !SlowOut1));
  solver.add(z3::implies(ProceedOut2, !SlowOut2 && !StopOut2));
  solver.add(z3::implies(SlowOut2, !ProceedOut2 && !StopOut2));
  solver.add(z3::implies(StopOut2, !ProceedOut2 && !SlowOut2));
  solver.add(z3::implies(ProceedOut3, !SlowOut3 && !StopOut3));
  solver.add(z3::implies(SlowOut3, !ProceedOut3 && !StopOut3));
  solver.add(z3::implies(StopOut3, !ProceedOut3 && !SlowOut3));

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

  add_assumption(Tag::E, 2, "GE_ped1_present", ped1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped2_clear", ped2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped3_clear", ped3 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ1_clear", occ1 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ2_clear", occ2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ3_clear", occ3 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_fric1_ok", friction_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric2_ok", friction_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric3_ok", friction_ok3 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt1_ok", battery_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt2_ok", battery_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt3_ok", battery_ok3 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped1_implies_StopOut1", implies(ped1 == ctx.bv_val(1, 1), StopOut1));
  add_assumption(Tag::E, 2, "GE_ped2_implies_StopOut2", implies(ped2 == ctx.bv_val(1, 1), StopOut2));
  add_assumption(Tag::E, 2, "GE_ped3_implies_StopOut3", implies(ped3 == ctx.bv_val(1, 1), StopOut3));
  add_assumption(Tag::E, 2, "GE_StopOut1_implies_occ1", implies(StopOut1, occ1 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut2_implies_occ2", implies(StopOut2, occ2 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut3_implies_occ3", implies(StopOut3, occ3 == ctx.bv_val(1, 1)));

  // ΓA: addressee assumptions (locked)
  add_assumption(Tag::A, 1, "GA_cmd_PROCEED", traffic_command == bv8(PROCEED));
  //solver.add(traffic_command == bv8(PROCEED)); // instead of GA_cmd_PROCEED with act literal

  // ΓC: connectivity assumptions (locked)
  add_assumption(Tag::C, 0, "GC_cmd_PROCEED_implies_ProceedOut1", implies(traffic_command == bv8(PROCEED), ProceedOut1));
  add_assumption(Tag::C, 0, "GC_ProceedOut1_implies_ProceedOut2", implies(ProceedOut1, ProceedOut2));
  add_assumption(Tag::C, 0, "GC_ProceedOut2_implies_ProceedOut3", implies(ProceedOut2, ProceedOut3));
  add_assumption(Tag::C, 0, "GC_cmd_Stop_implies_StopOut1", implies(traffic_command == bv8(STOP), StopOut1));
  add_assumption(Tag::C, 0, "GC_StopOut1_implies_StopOut2", implies(StopOut1, StopOut2));
  add_assumption(Tag::C, 0, "GC_StopOut2_implies_StopOut3", implies(StopOut2, StopOut3));

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

  std::cout << "=====================================================" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';
  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';

  // ==============================================================
}


TEST_CASE("traffic_four_vehicles_resolver", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "traffic" / "extraffic4.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("extraffic4.v");
  instance.write_graphviz("extraffic4.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 11;
  constexpr unsigned BV_WIDTH_8 = 8;
  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  auto get = [&](const std::string& name) { return ev_edges[signal_name_to_edge_id.at(name)]; };
  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };
  auto is = [&](const z3::expr& s, uint8_t v) { return s == bv8(v); };

  constexpr uint8_t PROCEED = 0b00000000;
  constexpr uint8_t SLOW = 0b00000001;
  constexpr uint8_t STOP = 0b00000010;
  constexpr uint8_t IDLE = 0b00010000;
  constexpr uint8_t ALIGN = 0b00010001;
  constexpr uint8_t GAP = 0b00010010;
  constexpr uint8_t ACCEL = 0b00010011;
  constexpr uint8_t SYNC = 0b00010100;
  constexpr uint8_t CRUISE = 0b00010101;
  constexpr uint8_t CRAWL = 0b00010110;
  constexpr uint8_t HOLD = 0b00010111;
  constexpr uint8_t BRAKE = 0b00011000;

  z3::expr battery_ok1 = get("|D1_ComposedSystem_battery_ok1_time_10|");
  z3::expr friction_ok1 = get("|D1_ComposedSystem_friction_ok1_time_10|");
  z3::expr occ1 = get("|D1_ComposedSystem_occ1_time_10|");
  z3::expr ped1 = get("|D1_ComposedSystem_ped1_time_10|");
  z3::expr vehicle1_state = get("|D1_ComposedSystem_vehicle1_state_time_10|");

  z3::expr battery_ok2 = get("|D1_ComposedSystem_battery_ok2_time_10|");
  z3::expr friction_ok2 = get("|D1_ComposedSystem_friction_ok2_time_10|");
  z3::expr occ2 = get("|D1_ComposedSystem_occ2_time_10|");
  z3::expr ped2 = get("|D1_ComposedSystem_ped2_time_10|");
  z3::expr vehicle2_state = get("|D1_ComposedSystem_vehicle2_state_time_10|");

  z3::expr battery_ok3 = get("|D1_ComposedSystem_battery_ok3_time_10|");
  z3::expr friction_ok3 = get("|D1_ComposedSystem_friction_ok3_time_10|");
  z3::expr occ3 = get("|D1_ComposedSystem_occ3_time_10|");
  z3::expr ped3 = get("|D1_ComposedSystem_ped3_time_10|");
  z3::expr vehicle3_state = get("|D1_ComposedSystem_vehicle3_state_time_10|");

  z3::expr battery_ok4 = get("|D1_ComposedSystem_battery_ok4_time_10|");
  z3::expr friction_ok4 = get("|D1_ComposedSystem_friction_ok4_time_10|");
  z3::expr occ4 = get("|D1_ComposedSystem_occ4_time_10|");
  z3::expr ped4 = get("|D1_ComposedSystem_ped4_time_10|");
  z3::expr vehicle4_state = get("|D1_ComposedSystem_vehicle4_state_time_10|");

  z3::expr traffic_command = get("|D1_ComposedSystem_traffic_command_time_10|");

  z3::expr ProceedOut1 = is(vehicle1_state, ALIGN) || is(vehicle1_state, GAP) ||
                         is(vehicle1_state, ACCEL) || is(vehicle1_state, SYNC) ||
                         is(vehicle1_state, CRUISE);
  z3::expr SlowOut1 = is(vehicle1_state, CRAWL);
  z3::expr StopOut1 = is(vehicle1_state, BRAKE) || is(vehicle1_state, HOLD) || is(vehicle1_state, IDLE);

  z3::expr ProceedOut2 = is(vehicle2_state, ALIGN) || is(vehicle2_state, GAP) ||
                         is(vehicle2_state, ACCEL) || is(vehicle2_state, SYNC) ||
                         is(vehicle2_state, CRUISE);
  z3::expr SlowOut2 = is(vehicle2_state, CRAWL);
  z3::expr StopOut2 = is(vehicle2_state, BRAKE) || is(vehicle2_state, HOLD) || is(vehicle2_state, IDLE);

  z3::expr ProceedOut3 = is(vehicle3_state, ALIGN) || is(vehicle3_state, GAP) ||
                         is(vehicle3_state, ACCEL) || is(vehicle3_state, SYNC) ||
                         is(vehicle3_state, CRUISE);
  z3::expr SlowOut3 = is(vehicle3_state, CRAWL);
  z3::expr StopOut3 = is(vehicle3_state, BRAKE) || is(vehicle3_state, HOLD) || is(vehicle3_state, IDLE);

  z3::expr ProceedOut4 = is(vehicle4_state, ALIGN) || is(vehicle4_state, GAP) ||
                         is(vehicle4_state, ACCEL) || is(vehicle4_state, SYNC) ||
                         is(vehicle4_state, CRUISE);
  z3::expr SlowOut4 = is(vehicle4_state, CRAWL);
  z3::expr StopOut4 = is(vehicle4_state, BRAKE) || is(vehicle4_state, HOLD) || is(vehicle4_state, IDLE);

  solver.add(z3::implies(ProceedOut1, !SlowOut1 && !StopOut1));
  solver.add(z3::implies(SlowOut1, !ProceedOut1 && !StopOut1));
  solver.add(z3::implies(StopOut1, !ProceedOut1 && !SlowOut1));
  solver.add(z3::implies(ProceedOut2, !SlowOut2 && !StopOut2));
  solver.add(z3::implies(SlowOut2, !ProceedOut2 && !StopOut2));
  solver.add(z3::implies(StopOut2, !ProceedOut2 && !SlowOut2));
  solver.add(z3::implies(ProceedOut3, !SlowOut3 && !StopOut3));
  solver.add(z3::implies(SlowOut3, !ProceedOut3 && !StopOut3));
  solver.add(z3::implies(StopOut3, !ProceedOut3 && !SlowOut3));
  solver.add(z3::implies(ProceedOut4, !SlowOut4 && !StopOut4));
  solver.add(z3::implies(SlowOut4, !ProceedOut4 && !StopOut4));
  solver.add(z3::implies(StopOut4, !ProceedOut4 && !SlowOut4));
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

  add_assumption(Tag::E, 2, "GE_ped1_present", ped1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped2_clear", ped2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped3_clear", ped3 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped4_clear", ped4 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ1_clear", occ1 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ2_clear", occ2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ3_clear", occ3 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ4_clear", occ4 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_fric1_ok", friction_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric2_ok", friction_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric3_ok", friction_ok3 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric4_ok", friction_ok4 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt1_ok", battery_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt2_ok", battery_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt3_ok", battery_ok3 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt4_ok", battery_ok4 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped1_implies_StopOut1", implies(ped1 == ctx.bv_val(1, 1), StopOut1));
  add_assumption(Tag::E, 2, "GE_ped2_implies_StopOut2", implies(ped2 == ctx.bv_val(1, 1), StopOut2));
  add_assumption(Tag::E, 2, "GE_ped3_implies_StopOut3", implies(ped3 == ctx.bv_val(1, 1), StopOut3));
  add_assumption(Tag::E, 2, "GE_ped4_implies_StopOut4", implies(ped4 == ctx.bv_val(1, 1), StopOut4));
  add_assumption(Tag::E, 2, "GE_StopOut1_implies_occ1", implies(StopOut1, occ1 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut2_implies_occ2", implies(StopOut2, occ2 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut3_implies_occ3", implies(StopOut3, occ3 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut4_implies_occ4", implies(StopOut4, occ4 == ctx.bv_val(1, 1)));

  add_assumption(Tag::A, 1, "GA_cmd_PROCEED", traffic_command == bv8(PROCEED));

  add_assumption(Tag::C, 0, "GC_cmd_PROCEED_implies_ProceedOut1", implies(traffic_command == bv8(PROCEED), ProceedOut1));
  add_assumption(Tag::C, 0, "GC_ProceedOut1_implies_ProceedOut2", implies(ProceedOut1, ProceedOut2));
  add_assumption(Tag::C, 0, "GC_ProceedOut2_implies_ProceedOut3", implies(ProceedOut2, ProceedOut3));
  add_assumption(Tag::C, 0, "GC_ProceedOut3_implies_ProceedOut4", implies(ProceedOut3, ProceedOut4));
  add_assumption(Tag::C, 0, "GC_cmd_Stop_implies_StopOut1", implies(traffic_command == bv8(STOP), StopOut1));
  add_assumption(Tag::C, 0, "GC_StopOut1_implies_StopOut2", implies(StopOut1, StopOut2));
  add_assumption(Tag::C, 0, "GC_StopOut2_implies_StopOut3", implies(StopOut2, StopOut3));
  add_assumption(Tag::C, 0, "GC_StopOut3_implies_StopOut4", implies(StopOut3, StopOut4));

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

  std::cout << "=====================================================" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';
  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';

  // ==============================================================
}


TEST_CASE("traffic_five_vehicles_resolver", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "traffic" / "extraffic5.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("extraffic5.v");
  instance.write_graphviz("extraffic5.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);

  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 11;
  constexpr unsigned BV_WIDTH_8 = 8;
  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  auto get = [&](const std::string& name) { return ev_edges[signal_name_to_edge_id.at(name)]; };
  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };
  auto is = [&](const z3::expr& s, uint8_t v) { return s == bv8(v); };

  constexpr uint8_t PROCEED = 0b00000000;
  constexpr uint8_t SLOW = 0b00000001;
  constexpr uint8_t STOP = 0b00000010;
  constexpr uint8_t IDLE = 0b00010000;
  constexpr uint8_t ALIGN = 0b00010001;
  constexpr uint8_t GAP = 0b00010010;
  constexpr uint8_t ACCEL = 0b00010011;
  constexpr uint8_t SYNC = 0b00010100;
  constexpr uint8_t CRUISE = 0b00010101;
  constexpr uint8_t CRAWL = 0b00010110;
  constexpr uint8_t HOLD = 0b00010111;
  constexpr uint8_t BRAKE = 0b00011000;

  z3::expr battery_ok1 = get("|D1_ComposedSystem_battery_ok1_time_10|");
  z3::expr friction_ok1 = get("|D1_ComposedSystem_friction_ok1_time_10|");
  z3::expr occ1 = get("|D1_ComposedSystem_occ1_time_10|");
  z3::expr ped1 = get("|D1_ComposedSystem_ped1_time_10|");
  z3::expr vehicle1_state = get("|D1_ComposedSystem_vehicle1_state_time_10|");

  z3::expr battery_ok2 = get("|D1_ComposedSystem_battery_ok2_time_10|");
  z3::expr friction_ok2 = get("|D1_ComposedSystem_friction_ok2_time_10|");
  z3::expr occ2 = get("|D1_ComposedSystem_occ2_time_10|");
  z3::expr ped2 = get("|D1_ComposedSystem_ped2_time_10|");
  z3::expr vehicle2_state = get("|D1_ComposedSystem_vehicle2_state_time_10|");

  z3::expr battery_ok3 = get("|D1_ComposedSystem_battery_ok3_time_10|");
  z3::expr friction_ok3 = get("|D1_ComposedSystem_friction_ok3_time_10|");
  z3::expr occ3 = get("|D1_ComposedSystem_occ3_time_10|");
  z3::expr ped3 = get("|D1_ComposedSystem_ped3_time_10|");
  z3::expr vehicle3_state = get("|D1_ComposedSystem_vehicle3_state_time_10|");

  z3::expr battery_ok4 = get("|D1_ComposedSystem_battery_ok4_time_10|");
  z3::expr friction_ok4 = get("|D1_ComposedSystem_friction_ok4_time_10|");
  z3::expr occ4 = get("|D1_ComposedSystem_occ4_time_10|");
  z3::expr ped4 = get("|D1_ComposedSystem_ped4_time_10|");
  z3::expr vehicle4_state = get("|D1_ComposedSystem_vehicle4_state_time_10|");

  z3::expr battery_ok5 = get("|D1_ComposedSystem_battery_ok5_time_10|");
  z3::expr friction_ok5 = get("|D1_ComposedSystem_friction_ok5_time_10|");
  z3::expr occ5 = get("|D1_ComposedSystem_occ5_time_10|");
  z3::expr ped5 = get("|D1_ComposedSystem_ped5_time_10|");
  z3::expr vehicle5_state = get("|D1_ComposedSystem_vehicle5_state_time_10|");

  z3::expr traffic_command = get("|D1_ComposedSystem_traffic_command_time_10|");

  z3::expr ProceedOut1 = is(vehicle1_state, ALIGN) || is(vehicle1_state, GAP) ||
                         is(vehicle1_state, ACCEL) || is(vehicle1_state, SYNC) ||
                         is(vehicle1_state, CRUISE);
  z3::expr SlowOut1 = is(vehicle1_state, CRAWL);
  z3::expr StopOut1 = is(vehicle1_state, BRAKE) || is(vehicle1_state, HOLD) || is(vehicle1_state, IDLE);

  z3::expr ProceedOut2 = is(vehicle2_state, ALIGN) || is(vehicle2_state, GAP) ||
                         is(vehicle2_state, ACCEL) || is(vehicle2_state, SYNC) ||
                         is(vehicle2_state, CRUISE);
  z3::expr SlowOut2 = is(vehicle2_state, CRAWL);
  z3::expr StopOut2 = is(vehicle2_state, BRAKE) || is(vehicle2_state, HOLD) || is(vehicle2_state, IDLE);

  z3::expr ProceedOut3 = is(vehicle3_state, ALIGN) || is(vehicle3_state, GAP) ||
                         is(vehicle3_state, ACCEL) || is(vehicle3_state, SYNC) ||
                         is(vehicle3_state, CRUISE);
  z3::expr SlowOut3 = is(vehicle3_state, CRAWL);
  z3::expr StopOut3 = is(vehicle3_state, BRAKE) || is(vehicle3_state, HOLD) || is(vehicle3_state, IDLE);

  z3::expr ProceedOut4 = is(vehicle4_state, ALIGN) || is(vehicle4_state, GAP) ||
                         is(vehicle4_state, ACCEL) || is(vehicle4_state, SYNC) ||
                         is(vehicle4_state, CRUISE);
  z3::expr SlowOut4 = is(vehicle4_state, CRAWL);
  z3::expr StopOut4 = is(vehicle4_state, BRAKE) || is(vehicle4_state, HOLD) || is(vehicle4_state, IDLE);

  z3::expr ProceedOut5 = is(vehicle5_state, ALIGN) || is(vehicle5_state, GAP) ||
                         is(vehicle5_state, ACCEL) || is(vehicle5_state, SYNC) ||
                         is(vehicle5_state, CRUISE);
  z3::expr SlowOut5 = is(vehicle5_state, CRAWL);
  z3::expr StopOut5 = is(vehicle5_state, BRAKE) || is(vehicle5_state, HOLD) || is(vehicle5_state, IDLE);

  solver.add(z3::implies(ProceedOut1, !SlowOut1 && !StopOut1));
  solver.add(z3::implies(SlowOut1, !ProceedOut1 && !StopOut1));
  solver.add(z3::implies(StopOut1, !ProceedOut1 && !SlowOut1));
  solver.add(z3::implies(ProceedOut2, !SlowOut2 && !StopOut2));
  solver.add(z3::implies(SlowOut2, !ProceedOut2 && !StopOut2));
  solver.add(z3::implies(StopOut2, !ProceedOut2 && !SlowOut2));
  solver.add(z3::implies(ProceedOut3, !SlowOut3 && !StopOut3));
  solver.add(z3::implies(SlowOut3, !ProceedOut3 && !StopOut3));
  solver.add(z3::implies(StopOut3, !ProceedOut3 && !SlowOut3));
  solver.add(z3::implies(ProceedOut4, !SlowOut4 && !StopOut4));
  solver.add(z3::implies(SlowOut4, !ProceedOut4 && !StopOut4));
  solver.add(z3::implies(StopOut4, !ProceedOut4 && !SlowOut4));
  solver.add(z3::implies(ProceedOut5, !SlowOut5 && !StopOut5));
  solver.add(z3::implies(SlowOut5, !ProceedOut5 && !StopOut5));
  solver.add(z3::implies(StopOut5, !ProceedOut5 && !SlowOut5));
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

  add_assumption(Tag::E, 2, "GE_ped1_present", ped1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped2_clear", ped2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped3_clear", ped3 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped4_clear", ped4 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped5_clear", ped5 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ1_clear", occ1 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ2_clear", occ2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ3_clear", occ3 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ4_clear", occ4 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ5_clear", occ5 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_fric1_ok", friction_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric2_ok", friction_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric3_ok", friction_ok3 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric4_ok", friction_ok4 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric5_ok", friction_ok5 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt1_ok", battery_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt2_ok", battery_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt3_ok", battery_ok3 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt4_ok", battery_ok4 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt5_ok", battery_ok5 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped1_implies_StopOut1", implies(ped1 == ctx.bv_val(1, 1), StopOut1));
  add_assumption(Tag::E, 2, "GE_ped2_implies_StopOut2", implies(ped2 == ctx.bv_val(1, 1), StopOut2));
  add_assumption(Tag::E, 2, "GE_ped3_implies_StopOut3", implies(ped3 == ctx.bv_val(1, 1), StopOut3));
  add_assumption(Tag::E, 2, "GE_ped4_implies_StopOut4", implies(ped4 == ctx.bv_val(1, 1), StopOut4));
  add_assumption(Tag::E, 2, "GE_ped5_implies_StopOut5", implies(ped5 == ctx.bv_val(1, 1), StopOut5));
  add_assumption(Tag::E, 2, "GE_StopOut1_implies_occ1", implies(StopOut1, occ1 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut2_implies_occ2", implies(StopOut2, occ2 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut3_implies_occ3", implies(StopOut3, occ3 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut4_implies_occ4", implies(StopOut4, occ4 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut5_implies_occ5", implies(StopOut5, occ5 == ctx.bv_val(1, 1)));

  add_assumption(Tag::A, 1, "GA_cmd_PROCEED", traffic_command == bv8(PROCEED));

  add_assumption(Tag::C, 0, "GC_cmd_PROCEED_implies_ProceedOut1", implies(traffic_command == bv8(PROCEED), ProceedOut1));
  add_assumption(Tag::C, 0, "GC_ProceedOut1_implies_ProceedOut2", implies(ProceedOut1, ProceedOut2));
  add_assumption(Tag::C, 0, "GC_ProceedOut2_implies_ProceedOut3", implies(ProceedOut2, ProceedOut3));
  add_assumption(Tag::C, 0, "GC_ProceedOut3_implies_ProceedOut4", implies(ProceedOut3, ProceedOut4));
  add_assumption(Tag::C, 0, "GC_ProceedOut4_implies_ProceedOut5", implies(ProceedOut4, ProceedOut5));
  add_assumption(Tag::C, 0, "GC_cmd_Stop_implies_StopOut1", implies(traffic_command == bv8(STOP), StopOut1));
  add_assumption(Tag::C, 0, "GC_StopOut1_implies_StopOut2", implies(StopOut1, StopOut2));
  add_assumption(Tag::C, 0, "GC_StopOut2_implies_StopOut3", implies(StopOut2, StopOut3));
  add_assumption(Tag::C, 0, "GC_StopOut3_implies_StopOut4", implies(StopOut3, StopOut4));
  add_assumption(Tag::C, 0, "GC_StopOut4_implies_StopOut5", implies(StopOut4, StopOut5));

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

  std::cout << "=====================================================" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';
  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';

  // ==============================================================
}


TEST_CASE("traffic_ten_vehicles_resolver", "[Explainer][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "traffic" / "extraffic10.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_verilog("extraffic10.v");
  instance.write_graphviz("extraffic10.dot");

  z3::context ctx;
  z3::solver solver(ctx);
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);

  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;
  constexpr uint32_t unroll_factor = 11;
  constexpr unsigned BV_WIDTH_8 = 8;
  for (uint32_t i = 0; i < unroll_factor; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }

  auto get = [&](const std::string& name) { return ev_edges[signal_name_to_edge_id.at(name)]; };
  auto bv8 = [&](uint8_t v) { return ctx.bv_val(v, BV_WIDTH_8); };
  auto is = [&](const z3::expr& s, uint8_t v) { return s == bv8(v); };

  constexpr uint8_t PROCEED = 0b00000000;
  constexpr uint8_t SLOW = 0b00000001;
  constexpr uint8_t STOP = 0b00000010;
  constexpr uint8_t IDLE = 0b00010000;
  constexpr uint8_t ALIGN = 0b00010001;
  constexpr uint8_t GAP = 0b00010010;
  constexpr uint8_t ACCEL = 0b00010011;
  constexpr uint8_t SYNC = 0b00010100;
  constexpr uint8_t CRUISE = 0b00010101;
  constexpr uint8_t CRAWL = 0b00010110;
  constexpr uint8_t HOLD = 0b00010111;
  constexpr uint8_t BRAKE = 0b00011000;

  z3::expr battery_ok1 = get("|D1_ComposedSystem_battery_ok1_time_10|");
  z3::expr friction_ok1 = get("|D1_ComposedSystem_friction_ok1_time_10|");
  z3::expr occ1 = get("|D1_ComposedSystem_occ1_time_10|");
  z3::expr ped1 = get("|D1_ComposedSystem_ped1_time_10|");
  z3::expr vehicle1_state = get("|D1_ComposedSystem_vehicle1_state_time_10|");

  z3::expr battery_ok2 = get("|D1_ComposedSystem_battery_ok2_time_10|");
  z3::expr friction_ok2 = get("|D1_ComposedSystem_friction_ok2_time_10|");
  z3::expr occ2 = get("|D1_ComposedSystem_occ2_time_10|");
  z3::expr ped2 = get("|D1_ComposedSystem_ped2_time_10|");
  z3::expr vehicle2_state = get("|D1_ComposedSystem_vehicle2_state_time_10|");

  z3::expr battery_ok3 = get("|D1_ComposedSystem_battery_ok3_time_10|");
  z3::expr friction_ok3 = get("|D1_ComposedSystem_friction_ok3_time_10|");
  z3::expr occ3 = get("|D1_ComposedSystem_occ3_time_10|");
  z3::expr ped3 = get("|D1_ComposedSystem_ped3_time_10|");
  z3::expr vehicle3_state = get("|D1_ComposedSystem_vehicle3_state_time_10|");

  z3::expr battery_ok4 = get("|D1_ComposedSystem_battery_ok4_time_10|");
  z3::expr friction_ok4 = get("|D1_ComposedSystem_friction_ok4_time_10|");
  z3::expr occ4 = get("|D1_ComposedSystem_occ4_time_10|");
  z3::expr ped4 = get("|D1_ComposedSystem_ped4_time_10|");
  z3::expr vehicle4_state = get("|D1_ComposedSystem_vehicle4_state_time_10|");

  z3::expr battery_ok5 = get("|D1_ComposedSystem_battery_ok5_time_10|");
  z3::expr friction_ok5 = get("|D1_ComposedSystem_friction_ok5_time_10|");
  z3::expr occ5 = get("|D1_ComposedSystem_occ5_time_10|");
  z3::expr ped5 = get("|D1_ComposedSystem_ped5_time_10|");
  z3::expr vehicle5_state = get("|D1_ComposedSystem_vehicle5_state_time_10|");

  z3::expr battery_ok6 = get("|D1_ComposedSystem_battery_ok6_time_10|");
  z3::expr friction_ok6 = get("|D1_ComposedSystem_friction_ok6_time_10|");
  z3::expr occ6 = get("|D1_ComposedSystem_occ6_time_10|");
  z3::expr ped6 = get("|D1_ComposedSystem_ped6_time_10|");
  z3::expr vehicle6_state = get("|D1_ComposedSystem_vehicle6_state_time_10|");

  z3::expr battery_ok7 = get("|D1_ComposedSystem_battery_ok7_time_10|");
  z3::expr friction_ok7 = get("|D1_ComposedSystem_friction_ok7_time_10|");
  z3::expr occ7 = get("|D1_ComposedSystem_occ7_time_10|");
  z3::expr ped7 = get("|D1_ComposedSystem_ped7_time_10|");
  z3::expr vehicle7_state = get("|D1_ComposedSystem_vehicle7_state_time_10|");

  z3::expr battery_ok8 = get("|D1_ComposedSystem_battery_ok8_time_10|");
  z3::expr friction_ok8 = get("|D1_ComposedSystem_friction_ok8_time_10|");
  z3::expr occ8 = get("|D1_ComposedSystem_occ8_time_10|");
  z3::expr ped8 = get("|D1_ComposedSystem_ped8_time_10|");
  z3::expr vehicle8_state = get("|D1_ComposedSystem_vehicle8_state_time_10|");

  z3::expr battery_ok9 = get("|D1_ComposedSystem_battery_ok9_time_10|");
  z3::expr friction_ok9 = get("|D1_ComposedSystem_friction_ok9_time_10|");
  z3::expr occ9 = get("|D1_ComposedSystem_occ9_time_10|");
  z3::expr ped9 = get("|D1_ComposedSystem_ped9_time_10|");
  z3::expr vehicle9_state = get("|D1_ComposedSystem_vehicle9_state_time_10|");

  z3::expr battery_ok10 = get("|D1_ComposedSystem_battery_ok10_time_10|");
  z3::expr friction_ok10 = get("|D1_ComposedSystem_friction_ok10_time_10|");
  z3::expr occ10 = get("|D1_ComposedSystem_occ10_time_10|");
  z3::expr ped10 = get("|D1_ComposedSystem_ped10_time_10|");
  z3::expr vehicle10_state = get("|D1_ComposedSystem_vehicle10_state_time_10|");

  z3::expr traffic_command = get("|D1_ComposedSystem_traffic_command_time_10|");

  z3::expr ProceedOut1 = is(vehicle1_state, ALIGN) || is(vehicle1_state, GAP) ||
                         is(vehicle1_state, ACCEL) || is(vehicle1_state, SYNC) ||
                         is(vehicle1_state, CRUISE);
  z3::expr SlowOut1 = is(vehicle1_state, CRAWL);
  z3::expr StopOut1 = is(vehicle1_state, BRAKE) || is(vehicle1_state, HOLD) || is(vehicle1_state, IDLE);

  z3::expr ProceedOut2 = is(vehicle2_state, ALIGN) || is(vehicle2_state, GAP) ||
                         is(vehicle2_state, ACCEL) || is(vehicle2_state, SYNC) ||
                         is(vehicle2_state, CRUISE);
  z3::expr SlowOut2 = is(vehicle2_state, CRAWL);
  z3::expr StopOut2 = is(vehicle2_state, BRAKE) || is(vehicle2_state, HOLD) || is(vehicle2_state, IDLE);

  z3::expr ProceedOut3 = is(vehicle3_state, ALIGN) || is(vehicle3_state, GAP) ||
                         is(vehicle3_state, ACCEL) || is(vehicle3_state, SYNC) ||
                         is(vehicle3_state, CRUISE);
  z3::expr SlowOut3 = is(vehicle3_state, CRAWL);
  z3::expr StopOut3 = is(vehicle3_state, BRAKE) || is(vehicle3_state, HOLD) || is(vehicle3_state, IDLE);

  z3::expr ProceedOut4 = is(vehicle4_state, ALIGN) || is(vehicle4_state, GAP) ||
                         is(vehicle4_state, ACCEL) || is(vehicle4_state, SYNC) ||
                         is(vehicle4_state, CRUISE);
  z3::expr SlowOut4 = is(vehicle4_state, CRAWL);
  z3::expr StopOut4 = is(vehicle4_state, BRAKE) || is(vehicle4_state, HOLD) || is(vehicle4_state, IDLE);

  z3::expr ProceedOut5 = is(vehicle5_state, ALIGN) || is(vehicle5_state, GAP) ||
                         is(vehicle5_state, ACCEL) || is(vehicle5_state, SYNC) ||
                         is(vehicle5_state, CRUISE);
  z3::expr SlowOut5 = is(vehicle5_state, CRAWL);
  z3::expr StopOut5 = is(vehicle5_state, BRAKE) || is(vehicle5_state, HOLD) || is(vehicle5_state, IDLE);

  z3::expr ProceedOut6 = is(vehicle6_state, ALIGN) || is(vehicle6_state, GAP) ||
                         is(vehicle6_state, ACCEL) || is(vehicle6_state, SYNC) ||
                         is(vehicle6_state, CRUISE);
  z3::expr SlowOut6 = is(vehicle6_state, CRAWL);
  z3::expr StopOut6 = is(vehicle6_state, BRAKE) || is(vehicle6_state, HOLD) || is(vehicle6_state, IDLE);

  z3::expr ProceedOut7 = is(vehicle7_state, ALIGN) || is(vehicle7_state, GAP) ||
                         is(vehicle7_state, ACCEL) || is(vehicle7_state, SYNC) ||
                         is(vehicle7_state, CRUISE);
  z3::expr SlowOut7 = is(vehicle7_state, CRAWL);
  z3::expr StopOut7 = is(vehicle7_state, BRAKE) || is(vehicle7_state, HOLD) || is(vehicle7_state, IDLE);

  z3::expr ProceedOut8 = is(vehicle8_state, ALIGN) || is(vehicle8_state, GAP) ||
                         is(vehicle8_state, ACCEL) || is(vehicle8_state, SYNC) ||
                         is(vehicle8_state, CRUISE);
  z3::expr SlowOut8 = is(vehicle8_state, CRAWL);
  z3::expr StopOut8 = is(vehicle8_state, BRAKE) || is(vehicle8_state, HOLD) || is(vehicle8_state, IDLE);

  z3::expr ProceedOut9 = is(vehicle9_state, ALIGN) || is(vehicle9_state, GAP) ||
                         is(vehicle9_state, ACCEL) || is(vehicle9_state, SYNC) ||
                         is(vehicle9_state, CRUISE);
  z3::expr SlowOut9 = is(vehicle9_state, CRAWL);
  z3::expr StopOut9 = is(vehicle9_state, BRAKE) || is(vehicle9_state, HOLD) || is(vehicle9_state, IDLE);

  z3::expr ProceedOut10 = is(vehicle10_state, ALIGN) || is(vehicle10_state, GAP) ||
                          is(vehicle10_state, ACCEL) || is(vehicle10_state, SYNC) ||
                          is(vehicle10_state, CRUISE);
  z3::expr SlowOut10 = is(vehicle10_state, CRAWL);
  z3::expr StopOut10 = is(vehicle10_state, BRAKE) || is(vehicle10_state, HOLD) || is(vehicle10_state, IDLE);

  solver.add(z3::implies(ProceedOut1, !SlowOut1 && !StopOut1));
  solver.add(z3::implies(SlowOut1, !ProceedOut1 && !StopOut1));
  solver.add(z3::implies(StopOut1, !ProceedOut1 && !SlowOut1));
  solver.add(z3::implies(ProceedOut2, !SlowOut2 && !StopOut2));
  solver.add(z3::implies(SlowOut2, !ProceedOut2 && !StopOut2));
  solver.add(z3::implies(StopOut2, !ProceedOut2 && !SlowOut2));
  solver.add(z3::implies(ProceedOut3, !SlowOut3 && !StopOut3));
  solver.add(z3::implies(SlowOut3, !ProceedOut3 && !StopOut3));
  solver.add(z3::implies(StopOut3, !ProceedOut3 && !SlowOut3));
  solver.add(z3::implies(ProceedOut4, !SlowOut4 && !StopOut4));
  solver.add(z3::implies(SlowOut4, !ProceedOut4 && !StopOut4));
  solver.add(z3::implies(StopOut4, !ProceedOut4 && !SlowOut4));
  solver.add(z3::implies(ProceedOut5, !SlowOut5 && !StopOut5));
  solver.add(z3::implies(SlowOut5, !ProceedOut5 && !StopOut5));
  solver.add(z3::implies(StopOut5, !ProceedOut5 && !SlowOut5));
  solver.add(z3::implies(ProceedOut6, !SlowOut6 && !StopOut6));
  solver.add(z3::implies(SlowOut6, !ProceedOut6 && !StopOut6));
  solver.add(z3::implies(StopOut6, !ProceedOut6 && !SlowOut6));
  solver.add(z3::implies(ProceedOut7, !SlowOut7 && !StopOut7));
  solver.add(z3::implies(SlowOut7, !ProceedOut7 && !StopOut7));
  solver.add(z3::implies(StopOut7, !ProceedOut7 && !SlowOut7));
  solver.add(z3::implies(ProceedOut8, !SlowOut8 && !StopOut8));
  solver.add(z3::implies(SlowOut8, !ProceedOut8 && !StopOut8));
  solver.add(z3::implies(StopOut8, !ProceedOut8 && !SlowOut8));
  solver.add(z3::implies(ProceedOut9, !SlowOut9 && !StopOut9));
  solver.add(z3::implies(SlowOut9, !ProceedOut9 && !StopOut9));
  solver.add(z3::implies(StopOut9, !ProceedOut9 && !SlowOut9));
  solver.add(z3::implies(ProceedOut10, !SlowOut10 && !StopOut10));
  solver.add(z3::implies(SlowOut10, !ProceedOut10 && !StopOut10));
  solver.add(z3::implies(StopOut10, !ProceedOut10 && !SlowOut10));

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

  add_assumption(Tag::E, 2, "GE_ped1_present", ped1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped2_clear", ped2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped3_clear", ped3 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped4_clear", ped4 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped5_clear", ped5 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped6_clear", ped6 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped7_clear", ped7 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped8_clear", ped8 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped9_clear", ped9 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_ped10_clear", ped10 == ctx.bv_val(0, 1));

  add_assumption(Tag::E, 2, "GE_occ1_clear", occ1 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ2_clear", occ2 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ3_clear", occ3 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ4_clear", occ4 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ5_clear", occ5 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ6_clear", occ6 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ7_clear", occ7 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ8_clear", occ8 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ9_clear", occ9 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_occ10_clear", occ10 == ctx.bv_val(0, 1));
  add_assumption(Tag::E, 2, "GE_fric1_ok", friction_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric2_ok", friction_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric3_ok", friction_ok3 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric4_ok", friction_ok4 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric5_ok", friction_ok5 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric6_ok", friction_ok6 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric7_ok", friction_ok7 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric8_ok", friction_ok8 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric9_ok", friction_ok9 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_fric10_ok", friction_ok10 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt1_ok", battery_ok1 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt2_ok", battery_ok2 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt3_ok", battery_ok3 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt4_ok", battery_ok4 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt5_ok", battery_ok5 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt6_ok", battery_ok6 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt7_ok", battery_ok7 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt8_ok", battery_ok8 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt9_ok", battery_ok9 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_batt10_ok", battery_ok10 == ctx.bv_val(1, 1));
  add_assumption(Tag::E, 2, "GE_ped1_implies_StopOut1", implies(ped1 == ctx.bv_val(1, 1), StopOut1));
  add_assumption(Tag::E, 2, "GE_ped2_implies_StopOut2", implies(ped2 == ctx.bv_val(1, 1), StopOut2));
  add_assumption(Tag::E, 2, "GE_ped3_implies_StopOut3", implies(ped3 == ctx.bv_val(1, 1), StopOut3));
  add_assumption(Tag::E, 2, "GE_ped4_implies_StopOut4", implies(ped4 == ctx.bv_val(1, 1), StopOut4));
  add_assumption(Tag::E, 2, "GE_ped5_implies_StopOut5", implies(ped5 == ctx.bv_val(1, 1), StopOut5));
  add_assumption(Tag::E, 2, "GE_ped6_implies_StopOut6", implies(ped6 == ctx.bv_val(1, 1), StopOut6));
  add_assumption(Tag::E, 2, "GE_ped7_implies_StopOut7", implies(ped7 == ctx.bv_val(1, 1), StopOut7));
  add_assumption(Tag::E, 2, "GE_ped8_implies_StopOut8", implies(ped8 == ctx.bv_val(1, 1), StopOut8));
  add_assumption(Tag::E, 2, "GE_ped9_implies_StopOut9", implies(ped9 == ctx.bv_val(1, 1), StopOut9));
  add_assumption(Tag::E, 2, "GE_ped10_implies_StopOut10", implies(ped10 == ctx.bv_val(1, 1), StopOut10));

  add_assumption(Tag::E, 2, "GE_StopOut1_implies_occ1", implies(StopOut1, occ1 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut2_implies_occ2", implies(StopOut2, occ2 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut3_implies_occ3", implies(StopOut3, occ3 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut4_implies_occ4", implies(StopOut4, occ4 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut5_implies_occ5", implies(StopOut5, occ5 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut6_implies_occ6", implies(StopOut6, occ6 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut7_implies_occ7", implies(StopOut7, occ7 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut8_implies_occ8", implies(StopOut8, occ8 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut9_implies_occ9", implies(StopOut9, occ9 == ctx.bv_val(1, 1)));
  add_assumption(Tag::E, 2, "GE_StopOut10_implies_occ10", implies(StopOut10, occ10 == ctx.bv_val(1, 1)));

  add_assumption(Tag::A, 1, "GA_cmd_PROCEED", traffic_command == bv8(PROCEED));

  add_assumption(Tag::C, 0, "GC_cmd_PROCEED_implies_ProceedOut1", implies(traffic_command == bv8(PROCEED), ProceedOut1));
  add_assumption(Tag::C, 0, "GC_ProceedOut1_implies_ProceedOut2", implies(ProceedOut1, ProceedOut2));
  add_assumption(Tag::C, 0, "GC_ProceedOut2_implies_ProceedOut3", implies(ProceedOut2, ProceedOut3));
  add_assumption(Tag::C, 0, "GC_ProceedOut3_implies_ProceedOut4", implies(ProceedOut3, ProceedOut4));
  add_assumption(Tag::C, 0, "GC_ProceedOut4_implies_ProceedOut5", implies(ProceedOut4, ProceedOut5));
  add_assumption(Tag::C, 0, "GC_ProceedOut5_implies_ProceedOut6", implies(ProceedOut5, ProceedOut6));
  add_assumption(Tag::C, 0, "GC_ProceedOut6_implies_ProceedOut7", implies(ProceedOut6, ProceedOut7));
  add_assumption(Tag::C, 0, "GC_ProceedOut7_implies_ProceedOut8", implies(ProceedOut7, ProceedOut8));
  add_assumption(Tag::C, 0, "GC_ProceedOut8_implies_ProceedOut9", implies(ProceedOut8, ProceedOut9));
  add_assumption(Tag::C, 0, "GC_ProceedOut9_implies_ProceedOut10", implies(ProceedOut9, ProceedOut10));

  add_assumption(Tag::C, 0, "GC_cmd_Stop_implies_StopOut1", implies(traffic_command == bv8(STOP), StopOut1));
  add_assumption(Tag::C, 0, "GC_StopOut1_implies_StopOut2", implies(StopOut1, StopOut2));
  add_assumption(Tag::C, 0, "GC_StopOut2_implies_StopOut3", implies(StopOut2, StopOut3));
  add_assumption(Tag::C, 0, "GC_StopOut3_implies_StopOut4", implies(StopOut3, StopOut4));
  add_assumption(Tag::C, 0, "GC_StopOut4_implies_StopOut5", implies(StopOut4, StopOut5));
  add_assumption(Tag::C, 0, "GC_StopOut5_implies_StopOut6", implies(StopOut5, StopOut6));
  add_assumption(Tag::C, 0, "GC_StopOut6_implies_StopOut7", implies(StopOut6, StopOut7));
  add_assumption(Tag::C, 0, "GC_StopOut7_implies_StopOut8", implies(StopOut7, StopOut8));
  add_assumption(Tag::C, 0, "GC_StopOut8_implies_StopOut9", implies(StopOut8, StopOut9));
  add_assumption(Tag::C, 0, "GC_StopOut9_implies_StopOut10", implies(StopOut9, StopOut10));

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

  std::cout << "=====================================================" << '\n';
  std::cout << "Unroll bound (K): " << unroll_factor << '\n';
  std::cout << "Execution Time (wall clock): " << elapsed_time.count() << " sec" << '\n';
  std::cout << "User CPU time: " << (cpu_end - cpu_start) << " sec" << '\n';
  std::cout << "Peak Memory Usage: " << mem_end << " KB" << '\n';

  // ==============================================================
}