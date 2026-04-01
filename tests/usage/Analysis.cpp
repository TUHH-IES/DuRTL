//
// Created by moham on 11/13/2025.
//

#include "ducode/ExpLib/Analysis.h"

#include <bits/types/struct_rusage.h>
#include <sys/resource.h>
#include <z3++.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <z3_api.h>

double get_user_cpu_time_sec() {
  rusage usage{};
  getrusage(RUSAGE_SELF, &usage);

  auto user_time =
      std::chrono::seconds(usage.ru_utime.tv_sec) +
      std::chrono::microseconds(usage.ru_utime.tv_usec);

  return std::chrono::duration<double>(user_time).count();
}

int64_t get_memory_kb() {
  rusage usage{};
  getrusage(RUSAGE_SELF, &usage);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
  return usage.ru_maxrss;
}

// Function 1: Collect SMT variables from a single expression
void collect_vars(const z3::expr& e, std::unordered_set<Z3_ast>& seen) {
  if (e.is_const() && e.decl().decl_kind() == Z3_OP_UNINTERPRETED) {
    seen.insert(e);
  }
  for (unsigned i = 0; i < e.num_args(); ++i) {
    collect_vars(e.arg(i), seen);
  }
}

// Function 2: Collect SMT variables from a vector of expressions
std::unordered_set<Z3_ast> collect_vars_from_vector(const z3::expr_vector& ev) {
  std::unordered_set<Z3_ast> vars;
  for (const auto& expr: ev) {
    collect_vars(expr, vars);
  }
  return vars;
}

// Function 3: Count assigned variables in a model
std::unordered_set<std::string> collect_model_vars(const z3::model& m) {
  std::unordered_set<std::string> names;
  for (int i = 0; i < static_cast<int>(m.size()); ++i) {
    names.insert(m[i].name().str());
  }
  return names;
}

void collect_expr_vars(const z3::expr& e, std::unordered_set<Z3_ast>& seen) {
  if (seen.contains(e)) {
    return;
  }
  seen.insert(e);
  if (e.is_app()) {
    auto decl = e.decl();
    if (e.is_const() && decl.arity() == 0) {
      return;
    }
    for (const auto& arg: e.args()) {
      collect_expr_vars(arg, seen);
    }
  } else if (e.is_quantifier()) {
    collect_expr_vars(e.body(), seen);
  }
}

// Function 4: Collect SMT variables from solver assertions
std::unordered_set<Z3_ast> collect_vars_from_solver_assertions(const z3::solver& solver) {
  std::unordered_set<Z3_ast> vars;
  z3::expr_vector assertions = solver.assertions();
  for (const auto& assertion: assertions) {
    collect_expr_vars(assertion, vars);
  }
  return vars;
}