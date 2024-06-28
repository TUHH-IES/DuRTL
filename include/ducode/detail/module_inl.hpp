/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/all_cells.hpp>

inline void ducode::Module::add_cell(const std::unordered_set<std::string>& modules, const std::string& name, const std::string& type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes) {
  std::set<std::string> basic_binary_gates = {"$add", "$sub", "$mul", "$div", "$eq", "$eqx", "$ge", "$gt", "$le", "$lt", "$mod", "$ne", "$nex", "$pow", "$shl", "$shr", "$sshl", "$sshr"};
  std::set<std::string> basic_unary_gates = {"$not", "$logic_not", "$pos", "$neg"};


  //////////// BINARY OPERATORS ///////////////
  if (type == "$and") {
    m_cells.emplace<CellAnd>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$or") {
    m_cells.emplace<CellOr>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$xor") {
    m_cells.emplace<CellXor>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$xnor") {
    m_cells.emplace<CellXnor>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$shift") {
    m_cells.emplace<CellShift>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$shiftx") {
    m_cells.emplace<CellShiftx>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$logic_and") {
    m_cells.emplace<CellLogicAnd>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$logic_or") {
    m_cells.emplace<CellLogicOr>(name, type, ports, hidden, parameters, attributes);
  } else if (basic_binary_gates.contains(type)) {
    m_cells.emplace<CellBasicGateBinary>(name, type, ports, hidden, parameters, attributes);
    //////////// UNARY OPERATORS ///////////////
  } else if (basic_unary_gates.contains(type)) {
    m_cells.emplace<CellBasicGateUnary>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$reduce_and") {
    m_cells.emplace<CellReduceAnd>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$reduce_or" || type == "$reduce_bool") {//functionally the same as $reduce_or according to yosys manual; so a $reduce_bool type creates a 'CellReduceOr'
    m_cells.emplace<CellReduceOr>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$reduce_xor") {
    m_cells.emplace<CellReduceXor>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$reduce_xnor") {
    m_cells.emplace<CellReduceXnor>(name, type, ports, hidden, parameters, attributes);
    //////////// MULTIPLEXER ///////////////
  } else if (type == "$mux") {
    m_cells.emplace<CellMux>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$pmux") {
    m_cells.emplace<CellPMux>(name, type, ports, hidden, parameters, attributes);
    //////////// REGISTERS (DFF) ///////////////
  } else if (type == "$sr") {
    m_cells.emplace<CellSR>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dff") {
    m_cells.emplace<CellDFF>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$adff") {
    m_cells.emplace<CellADFF>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$sdff") {
    m_cells.emplace<CellSDFF>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$aldff") {
    m_cells.emplace<CellALDFF>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dffsr") {
    m_cells.emplace<CellDFFSR>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dffe") {
    m_cells.emplace<CellDFFE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$adffe") {
    m_cells.emplace<CellADFFE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$sdffe") {
    m_cells.emplace<CellSDFFE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$aldffe") {
    m_cells.emplace<CellALDFFE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dffsre") {
    m_cells.emplace<CellDFFSRE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dlatch") {
    m_cells.emplace<CellDLatch>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$adlatch") {
    m_cells.emplace<CellADLatch>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dlatchsr") {
    m_cells.emplace<CellDLatchSR>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$print") {
    m_cells.emplace<Print>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$mem_v2") {
    m_cells.emplace<MemV2>(name, type, ports, hidden, parameters, attributes);
  } else {
    if (modules.contains(type)) {
      spdlog::debug("CellType: Module ({})", type);
      m_cells.emplace<CellModule>(legalize_identifier(name), type, ports, hidden, parameters, attributes);
    } else {
      throw std::runtime_error(fmt::format("The type is not a Cell or a Module: {}", type));
    }
  }
}
