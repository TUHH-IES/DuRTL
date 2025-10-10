#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/utility/smt.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <z3++.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace ducode {

/**
   * @brief QifaSmt performs quantitative information flow anylsis using an SMT solver.
   *
   * Qifa determines for a given "secret" signal into how many equivalence classes the values of that secret fall when being 
   * only able to observe the output. Inputs and outputs of the top module are use for controlling a leak or observing the equivalence classes
   * for different secrets. 
   *
   * The analysis uses an unrolling of up to maxEquivalenceClasses instances where 
   *   (1) initial states and input sequences are forced to be identical
   *   (2) secrets are forced to be pairwise different
   *   (3) output sequences are forced to be pairwise different
   *
   * Assumptions:
   *   Secrets are assumed to be inputs, too. They are removed from the list of inputs for constraining the unrolled
   *   instances.
   *
   * Parameters: 
   *   clock -- string - clock signal of the design
   *   reset -- string - reset signal
   *   secrets -- list of stirngs - list of primary inputs considered to be secrets
   *   unroll_factor -- int - length of unrolling
   *   intermediate_step -- bool - true uses the optional intermediate check, that is more efficiently finding more equivalence classes under a fixed input sequence
   *   
   * IMPORTANT: This is a stateful class that keeps the SMT solver, its context and previously found information about equivalence classes.
   *
   * Plans: 
   *  - Add x-value simulation to generalize input/output/secret values.
   *  - Determine the number of values in a given equivalence classe.
   *  - Determine whether other input/output traces also belong to the same equivalence class.
   *  - Refactor - many classes and utility functions are within this class
   *
   * @param parmams - contains a vector<string> of names of secrets (which  
  */
class QifaSmt {
private:
  static z3::expr get_expr(uint32_t instance_id, const std::vector<std::string> &hier, const std::string &signalName, uint32_t time, std::unordered_map<std::string, uint32_t> &signal_name_to_expr_id, const z3::expr_vector &ev_edges) {
    std::string s_smt = create_smt_signal_name(instance_id, hier, signalName, time);
    auto itexpr = signal_name_to_expr_id.find(s_smt);
    assert(itexpr != signal_name_to_expr_id.end());
    const z3::expr &expr = ev_edges[itexpr->second];
    return expr;
  }

  struct Portnames {
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<std::string> secrets;
    std::vector<SignalIdentifier> states;
  };


  struct SmtMapping {
    std::vector<z3::expr> expr_inputs;
    std::vector<z3::expr> expr_outputs;
    std::vector<z3::expr> expr_secrets;
    std::vector<z3::expr> expr_states;
  };

  class SmtValueType : public std::string {
  public:
    explicit SmtValueType(const std::string &expr) {
      assign(expr);
    }
    explicit SmtValueType(const z3::expr &expr) {
      assign(expr.to_string());
    }
  };

  struct SmtValues {
    std::vector<SmtValueType> val_inputs;
    std::vector<SmtValueType> val_outputs;
    std::vector<SmtValueType> val_secrets;
    std::vector<SmtValueType> val_states;
  };

  class InstTimeIOVals : public std::vector<std::vector<SmtValues>> {
    static void print(std::ostream &ostr, const std::vector<std::string> &names, const std::vector<SmtValueType> &vals, const std::vector<z3::expr> &exprs) {
      assert(names.size() == vals.size());
      exprs;
      for (uint32_t index = 0; index < names.size(); index++) {
        ostr << names[index] << "= "// <<exprs[index] <<"= "
             << vals[index] << " -- ";
      }
    }

    static void print(std::ostream &ostr, const std::vector<SignalIdentifier> &names, const std::vector<SmtValueType> &vals, const std::vector<z3::expr> &exprs) {
      assert(names.size() == vals.size());
      exprs;
      for (uint32_t index = 0; index < names.size(); index++) {
        std::string sep;
        for (const auto &level: names[index].hierarchy) {
          ostr << sep << level;
          sep = ".";
        }
        ostr << sep << names[index].name << "= "// <<(exprs.size()>index ? exprs[index] : " -- ") <<"= "
             << vals[index] << " -- ";
      }
    }

  public:
    void print(std::ostream &ostr, const Portnames &portnms, const std::vector<std::vector<SmtMapping>> &instanceTimeIOExpr) {
      ostr << "Assignment";
      for (uint32_t inst = 0; inst < size(); inst++) {
        const auto &timeIOVals = this->at(inst);
        std::vector<SmtMapping> timeIOExpr;
        if (instanceTimeIOExpr.size() > inst) {
          timeIOExpr = instanceTimeIOExpr[inst];
        }
        ostr << "\n  Instance " << inst;
        for (uint32_t time = 0; time < timeIOVals.size(); time++) {
          ostr << "\n    Time " << time << ": ";
          print(ostr, portnms.inputs, timeIOVals[time].val_inputs, timeIOExpr[time].expr_inputs);
          print(ostr, portnms.outputs, timeIOVals[time].val_outputs, timeIOExpr[time].expr_outputs);
          print(ostr, portnms.secrets, timeIOVals[time].val_secrets, timeIOExpr[time].expr_secrets);
          print(ostr, portnms.states, timeIOVals[time].val_states, timeIOExpr[time].expr_states);
        }
      }
      ostr << "\n";
    }
    std::string str(const Portnames &portnms, const std::vector<std::vector<SmtMapping>> &instanceTimeIOExpr) {
      std::stringstream sstr;
      this->print(sstr, portnms, instanceTimeIOExpr);
      return sstr.str();
    }
  };

  static SmtValueType get_value(z3::model &model, const z3::expr &expr) {
    const z3::context &ctx = expr.ctx();
    z3::expr expr_val = model.eval(expr);//x1, true);
    Z3_ast ast = expr_val;
    SmtValueType value{Z3_get_numeral_string(ctx, ast)};
    return value;
  }

  static void get_values(const std::vector<SmtMapping> &timeIOExpr, z3::model &model, std::vector<SmtValues> &timeIOVals) {
    for (const auto &ioExpr: timeIOExpr) {
      SmtValues vals;
      for (const auto &expr: ioExpr.expr_inputs) {
        vals.val_inputs.emplace_back(get_value(model, expr));
      }
      for (const auto &expr: ioExpr.expr_outputs) {
        vals.val_outputs.emplace_back(get_value(model, expr));
      }
      for (const auto &expr: ioExpr.expr_secrets) {
        SmtValueType value{get_value(model, expr)};
        vals.val_secrets.emplace_back(value);
      }
      for (const auto &expr: ioExpr.expr_states) {
        vals.val_states.emplace_back(get_value(model, expr));
      }
      timeIOVals.emplace_back(vals);
    }
  }

  static InstTimeIOVals get_values(const std::vector<std::vector<SmtMapping>> &instanceTimeIOExpr, z3::model &model) {
    InstTimeIOVals instTimeIOVals;
    for (const auto &timeIOExpr: instanceTimeIOExpr) {
      std::vector<SmtValues> timeIOVals;
      get_values(timeIOExpr, model, timeIOVals);
      instTimeIOVals.emplace_back(timeIOVals);
    }
    return instTimeIOVals;
  }

  std::vector<SmtMapping> create_unrolled_instance(z3::context &ctx, z3::solver &solver, uint32_t instance_id) {
    std::vector<std::string> hier{top_module_name};
    std::unordered_map<std::string, uint32_t> sn2eid;
    z3::expr_vector ev_edges(ctx);
    std::vector<SmtMapping> timeIOExpr;
    for (uint32_t time = 0; time < unroll_factor; time++) {
      instance.export_smt2(ctx, solver, ev_edges, sn2eid, instance_id, time);

      SmtMapping ioExpr;
      for (const auto &name: portnms.inputs) {
        ioExpr.expr_inputs.push_back(get_expr(instance_id, hier, name, time, sn2eid, ev_edges));
      }
      for (const auto &name: portnms.outputs) {
        ioExpr.expr_outputs.push_back(get_expr(instance_id, hier, name, time, sn2eid, ev_edges));
      }
      for (const auto &name: portnms.secrets) {
        ioExpr.expr_secrets.push_back(get_expr(instance_id, hier, name, time, sn2eid, ev_edges));
      }
      for (const auto &name: portnms.states) {
        ioExpr.expr_states.push_back(get_expr(instance_id, name.hierarchy, name.name, time, sn2eid, ev_edges));
      }
      timeIOExpr.push_back(ioExpr);
    }
    return timeIOExpr;
  }


  boost::filesystem::path temp_dir;
  std::string top_module_name;
  DesignInstance instance;
  Portnames portnms;
  z3::context ctxInstances;
  z3::solver solverInstances{ctxInstances};
  z3::context ctxIntermediate;
  z3::solver solverIntermediate{ctxIntermediate};
  z3::check_result z3result = z3::unknown;
  uint32_t instance_id = 0;// Current number of copies in the instance to identify equivalence classes
  uint32_t unroll_factor;
  std::vector<std::vector<SmtMapping>> instanceTimeIOExpr;
  InstTimeIOVals instTimeIOvals;
  bool intermediateStep = false;
  std::vector<SmtMapping> intermediateStepTimeIOExpr;


  /// Translates a given SMT-value into the context of exprTarget, matching the bitwidth of exprTarget.
  static z3::expr translate_value_to_constant(z3::expr &exprTarget, const std::string &val) {
    z3::context &ctx = exprTarget.ctx();
    Z3_sort z3sort = Z3_get_sort(ctx, static_cast<Z3_ast>(exprTarget));
    unsigned bv_size = Z3_get_bv_sort_size(ctx, z3sort);

    // Create value in new context
    z3::expr exprVal = ctx.bv_val(val.c_str(), bv_size);
    return exprVal;
  }

public:
  QifaSmt(const Design &design, nlohmann::json &params) : temp_dir(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()),
                                                          unroll_factor(params["unroll_factor"]) {

    Expects(params.contains("clock"));
    Expects(params.contains("reset"));
    Expects(params.contains("secrets"));
    Expects(params.contains("unroll_factor"));

    if (params.contains("intermediate_step")) {
      intermediateStep = true;
    }

    auto created = boost::filesystem::create_directories(temp_dir);
    assert(created);
    spdlog::debug("Temp dir: {}", temp_dir.string());

    // get top module name
    auto top_module = std::find_if(design.get_modules().begin(), design.get_modules().end(), [&](const auto &mod) {
      return mod.get_name() == design.get_top_module_name();
    });
    if (top_module == design.get_modules().end()) {
      throw std::runtime_error("Top module not found");
    }
    top_module_name = top_module->get_name();

    instance = DesignInstance::create_instance(design);

    // ****
    // find input and output ports and state storage
    // ****
    portnms.inputs = top_module->get_input_names();
    portnms.outputs = top_module->get_output_names();
    portnms.secrets = params["secrets"].get<std::vector<std::string>>();
    portnms.states = instance.get_all_flipflop_ids();

    // Remove any secret strings from the inputs vector
    for (const auto &secret: portnms.secrets) {
      portnms.inputs.erase(std::remove(portnms.inputs.begin(), portnms.inputs.end(), secret), portnms.inputs.end());
      for (auto it = portnms.states.begin(); it != portnms.states.end(); it++) {
        std::string longname;
        if (!it->hierarchy.empty()) {
          longname = std::accumulate(it->hierarchy.begin(), it->hierarchy.end(), std::string{"."});
          longname += ".";
        }
        longname += it->name;
        if (longname == secret) {//Will this work - erasing an iterator while iterating...
          portnms.states.erase(it);
        }
      }
    }

    // Setup SMT solver
    // enabling unsat core tracking
    z3::params z3params(ctxInstances);
    z3params.set("unsat_core", true);
    solverInstances.set(z3params);

    // enabling unsat core tracking
    z3::params z3paramsI(ctxIntermediate);
    z3paramsI.set("unsat_core", true);
    solverIntermediate.set(z3paramsI);
  }

  InstTimeIOVals find_equivalence_classes(const uint32_t numMaxEquivalenceClasses, nlohmann::json &params) {
    // Main equivalence class finding loop

    if (intermediateStep && instance_id == 0) {
      //create unrolled instance for intermediate step
      intermediateStepTimeIOExpr = create_unrolled_instance(ctxIntermediate, solverIntermediate, 0);
      // Fix secret to be constant over time
      for (size_t time = 1; time < unroll_factor; ++time) {
        for (size_t idex = 0; idex < intermediateStepTimeIOExpr[time].expr_secrets.size(); idex++) {
          solverIntermediate.add(intermediateStepTimeIOExpr[time].expr_secrets[idex] == intermediateStepTimeIOExpr[time - 1].expr_secrets[idex]);
        }
      }
    }

    while (instance_id < numMaxEquivalenceClasses) {
      spdlog::info("Main loop for instance {}", instance_id);

      // Intermediate step may skip some instances; we have to add them here
      while (instanceTimeIOExpr.size() <= instance_id) {
        // Export the design instance to SMT for this equivalence class
        uint32_t creating_instance_id = instanceTimeIOExpr.size();
        spdlog::info("Adding SMT instance {} for equivalence class analysis", creating_instance_id);
        std::vector<SmtMapping> timeIOExpr = create_unrolled_instance(ctxInstances, solverInstances, creating_instance_id);
        instanceTimeIOExpr.push_back(timeIOExpr);

        if (params["smt_debug"]) {
          std::string smt2_filename = (temp_dir / fmt::format("qifa_smt_instance_{}_beforeConstr.smt2", creating_instance_id)).string();
          write_to_smt2(smt2_filename, solverInstances);
        }

        // Add constraints for equivalence class analysis
        // Need only adding constraints for new instance wrt. to all previous instances
        // Secrets are constant over time per instance; add constraints for new instance
        for (size_t time = 1; time < unroll_factor; ++time) {
          for (size_t idex = 0; idex < instanceTimeIOExpr[creating_instance_id][time].expr_secrets.size(); idex++) {
            solverInstances.add(instanceTimeIOExpr[creating_instance_id][time].expr_secrets[idex] == instanceTimeIOExpr[creating_instance_id][time - 1].expr_secrets[idex]);
          }
        }
        // Secrets are pairwise different between instances
        for (size_t other = 0; other < creating_instance_id; ++other) {
          for (size_t idex = 0; idex < instanceTimeIOExpr[creating_instance_id][0].expr_secrets.size(); idex++) {
            solverInstances.add(instanceTimeIOExpr[other][0].expr_secrets[idex] != instanceTimeIOExpr[creating_instance_id][0].expr_secrets[idex]);
          }
        }

        if (instance_id == 0) {
          continue;
        }

        // Initial states are identical across all instances
        if (!portnms.states.empty()) {
          for (size_t idex = 0; idex < instanceTimeIOExpr[creating_instance_id][0].expr_states.size(); idex++) {
            solverInstances.add(instanceTimeIOExpr[0][0].expr_states[idex] == instanceTimeIOExpr[creating_instance_id][0].expr_states[idex]);
          }
        }

        // Input sequences are identical across all instances
        for (size_t time = 0; time < unroll_factor; ++time) {
          for (size_t idex = 0; idex < instanceTimeIOExpr[creating_instance_id][time].expr_inputs.size(); idex++) {
            solverInstances.add(instanceTimeIOExpr[0][time].expr_inputs[idex] == instanceTimeIOExpr[creating_instance_id][time].expr_inputs[idex]);
          }
        }

        // Output sequences are pairwise different between instances
        for (size_t other = 0; other < creating_instance_id; ++other) {
          z3::expr_vector output_diff_constraints(ctxInstances);
          for (size_t idex = 0; idex < instanceTimeIOExpr[creating_instance_id][0].expr_outputs.size(); idex++) {
            for (size_t time = 0; time < unroll_factor; ++time) {
              z3::expr exprOutT = instanceTimeIOExpr[creating_instance_id][time].expr_outputs[idex];
              z3::expr exprOutTother = instanceTimeIOExpr[other][time].expr_outputs[idex];
              z3::expr diff = (exprOutT != exprOutTother);
              output_diff_constraints.push_back(diff);
            }
          }
          assert(!output_diff_constraints.empty());
          solverInstances.add(z3::mk_or(output_diff_constraints));
        }

        if (params["smt_debug"]) {
          write_to_smt2((temp_dir / fmt::format("qifa_smt_instance_{}.smt2", creating_instance_id)).string(), solverInstances);
        }

        creating_instance_id++;
      }

      if (instance_id == 0) {
        instance_id++;
        continue;
      }

      // Check satisfiability
      z3result = solverInstances.check();

      if (z3result == z3::sat) {
        z3::model model = solverInstances.get_model();
        spdlog::info("Found equivalence classes for {} instances", instance_id + 1);

        instTimeIOvals = get_values(instanceTimeIOExpr, model);
        spdlog::info(instTimeIOvals.str(portnms, instanceTimeIOExpr));

        if (params["smt_debug"]) {
          write_to_smt2((temp_dir / fmt::format("qifa_smt_instance_{}_model.smt2", instance_id)).string(), model);
        }

        // Check in intermediate instance with single unrolling, whether we can find a new secret
        //    leading to different output under the same input sequence
        //    We keep the unrolled instance over independent intermediate checks
        //    We have to recreate the constraints on primary input sequence, initial state and secret values which may change
        //    While incrementing the number of equivalence classes in the intermediate instance, we incrementally solver the same instance.
        if (intermediateStep) {
          solverIntermediate.push();

          // Fix input values
          z3::expr_vector constrainToNewSecret(ctxIntermediate);
          for (size_t idex = 0; idex < intermediateStepTimeIOExpr[0].expr_inputs.size(); idex++) {
            for (size_t time = 0; time < unroll_factor; ++time) {
              std::string val = instTimeIOvals[0][time].val_inputs[idex];
              z3::expr exprVar = intermediateStepTimeIOExpr[time].expr_inputs[idex];
              z3::expr exprConstant = translate_value_to_constant(exprVar, val);
              constrainToNewSecret.push_back(exprVar == exprConstant);
            }
          }

          // Fix initial state
          for (size_t idex = 0; idex < intermediateStepTimeIOExpr[0].expr_states.size(); idex++) {
            std::string val = instTimeIOvals[0][0].val_states[idex];
            z3::expr exprVar = intermediateStepTimeIOExpr[0].expr_states[idex];
            z3::expr exprConstant = translate_value_to_constant(exprVar, val);
            constrainToNewSecret.push_back(exprVar == exprConstant);
          }

          // Secret must be different
          for (auto &timeIOvals: instTimeIOvals) {
            for (size_t idex = 0; idex < intermediateStepTimeIOExpr[0].expr_secrets.size(); idex++) {
              std::string val = timeIOvals[0].val_secrets[idex];
              z3::expr exprVar = intermediateStepTimeIOExpr[0].expr_secrets[idex];
              z3::expr exprConstant = translate_value_to_constant(exprVar, val);
              constrainToNewSecret.push_back(exprVar != exprConstant);
            }
          }
          solverIntermediate.add(z3::mk_and(constrainToNewSecret));

          // Output sequence must be different from any of those before
          for (auto &timeIOvals: instTimeIOvals) {
            z3::expr_vector diffSequence(ctxIntermediate);
            for (size_t idex = 0; idex < intermediateStepTimeIOExpr[0].expr_outputs.size(); idex++) {
              for (size_t time = 0; time < unroll_factor; ++time) {
                std::string val = timeIOvals[time].val_outputs[idex];
                z3::expr exprVar = intermediateStepTimeIOExpr[time].expr_outputs[idex];
                z3::expr exprConstant = translate_value_to_constant(exprVar, val);
                diffSequence.push_back(exprVar != exprConstant);
              }
            }
            solverIntermediate.add(z3::mk_or(diffSequence));
          }
          while (intermediateStep && z3result == z3::sat && instance_id < numMaxEquivalenceClasses) {

            if (params["smt_debug"]) {
              write_to_smt2((temp_dir / fmt::format("qifa_smt_intermediate_{}.smt2", instance_id)).string(), solverIntermediate);
            }

            z3result = solverIntermediate.check();
            instance_id++;
            if (z3result == z3::sat) {
              //Extract additional values
              z3::model model = solverIntermediate.get_model();

              std::vector<SmtValues> timeIOVals;
              get_values(intermediateStepTimeIOExpr, model, timeIOVals);
              instTimeIOvals.push_back(timeIOVals);
              spdlog::info("Intermediate check successful -- found {} classes", instTimeIOvals.size());
              spdlog::info(instTimeIOvals.str(portnms, instanceTimeIOExpr));

              {
                // Add constraints wrt. last solution
                auto inst = instTimeIOvals.size() - 1;
                // Secret must be different also from last one found
                z3::expr_vector constrainToNewSecret(ctxIntermediate);
                for (size_t idex = 0; idex < intermediateStepTimeIOExpr[0].expr_secrets.size(); idex++) {
                  std::string val = instTimeIOvals[inst][0].val_secrets[idex];
                  z3::expr exprVar = intermediateStepTimeIOExpr[0].expr_secrets[idex];
                  z3::expr exprConstant = translate_value_to_constant(exprVar, val);
                  constrainToNewSecret.push_back(exprVar != exprConstant);
                }
                solverIntermediate.add(z3::mk_and(constrainToNewSecret));

                // Output sequence must be different also from trace before
                z3::expr_vector diffSequence(ctxIntermediate);
                for (size_t idex = 0; idex < intermediateStepTimeIOExpr[0].expr_outputs.size(); idex++) {
                  for (size_t time = 0; time < unroll_factor; ++time) {
                    std::string val = instTimeIOvals[inst][time].val_outputs[idex];
                    z3::expr exprVar = intermediateStepTimeIOExpr[time].expr_outputs[idex];
                    z3::expr exprConstant = translate_value_to_constant(exprVar, val);
                    diffSequence.push_back(exprVar != exprConstant);
                  }
                }
                solverIntermediate.add(z3::mk_or(diffSequence));
              }

              if (params["smt_debug"]) {
                write_to_smt2((temp_dir / fmt::format("qifa_smt_instance_{}_model.smt2", instance_id)).string(), model);
              }

            } else if (z3result == z3::unsat) {
              spdlog::info("Intermediate check for {} failed", instance_id);
              break;
            } else {
              spdlog::error("SMT solver returned unknown result");
              throw std::runtime_error("SMT solver returned unknown result");
              break;
            }
          }
          solverIntermediate.pop();
        }
      } else if (z3result == z3::unsat) {
        spdlog::info("No more equivalence classes found at instance count {}", instance_id + 1);
        break;
      } else {
        spdlog::error("SMT solver returned unknown result");
        throw std::runtime_error("SMT solver returned unknown result");
        break;
      }
    }
    if (z3result == z3::unsat) {
      spdlog::info("There are {} equivalence classes", instance_id);
    } else {
      spdlog::info("There may be more equivalence classes than {}", instance_id);
    }
    return instTimeIOvals;
  }
};

}//namespace ducode
