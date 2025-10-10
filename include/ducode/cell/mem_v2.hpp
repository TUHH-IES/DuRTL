/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

#include <spdlog/spdlog.h>

#include <iostream>

namespace ducode {

class MemV2 : public Cell {
public:
  MemV2(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {

    std::ranges::for_each(ports, [&](const ducode::Port& port) { port_map.emplace(port.m_name, port); });
    abits = std::stoull(parameters.at("ABITS").get<std::string>(), nullptr, 2);
    init = parameters.at("INIT").get<std::string>();
    memid = parameters.at("MEMID").get<std::string>();
    offset = std::stoull(parameters.at("OFFSET").get<std::string>(), nullptr, 2);
    rd_arst_value = parameters.at("RD_ARST_VALUE").get<std::string>();
    rd_ce_over_srst = parameters.at("RD_CE_OVER_SRST").get<std::string>();
    std::istringstream(parameters.at("RD_CLK_ENABLE").get<std::string>()) >> rd_clk_enable;
    rd_clk_polarity = parameters.at("RD_CLK_POLARITY").get<std::string>();
    rd_collision_x_mask = parameters.at("RD_COLLISION_X_MASK").get<std::string>();
    rd_init_value = parameters.at("RD_INIT_VALUE").get<std::string>();
    rd_ports = std::stoull(parameters.at("RD_PORTS").get<std::string>(), nullptr, 2);
    rd_srst_value = parameters.at("RD_SRST_VALUE").get<std::string>();
    rd_transparency_mask = parameters.at("RD_TRANSPARENCY_MASK").get<std::string>();
    rd_wide_continuation = parameters.at("RD_WIDE_CONTINUATION").get<std::string>();
    size = std::stoull(parameters.at("SIZE").get<std::string>(), nullptr, 2);
    width = std::stoull(parameters.at("WIDTH").get<std::string>(), nullptr, 2);
    std::istringstream(parameters.at("WR_CLK_ENABLE").get<std::string>()) >> wr_clk_enable;
    wr_clk_polarity = parameters.at("WR_CLK_POLARITY").get<std::string>();
    wr_ports = std::stoull(parameters.at("WR_PORTS").get<std::string>(), nullptr, 2);
    wr_priority_mask = parameters.at("WR_PRIORITY_MASK").get<std::string>();
    wr_wide_continuation = parameters.at("WR_WIDE_CONTINUATION").get<std::string>();

    Expects(port_map.at("RD_ADDR").m_bits.size() == (abits * rd_ports));
    Expects(port_map.at("RD_DATA").m_bits.size() == (width * rd_ports));
    Expects(port_map.at("WR_ADDR").m_bits.size() == (abits * wr_ports));
    Expects(port_map.at("WR_DATA").m_bits.size() == (width * wr_ports));
    Expects(port_map.at("WR_EN").m_bits.size() == (width * wr_ports));
  }

  std::unordered_map<std::string, ducode::Port> port_map;
  uint64_t abits = 0;
  std::string init;
  std::string memid;
  uint64_t offset = 0;
  std::string rd_arst_value;
  std::string rd_ce_over_srst;
  bool rd_clk_enable = false;
  std::string rd_clk_polarity;
  std::string rd_collision_x_mask;
  std::string rd_init_value;
  uint64_t rd_ports = 0;
  std::string rd_srst_value;
  std::string rd_transparency_mask;
  std::string rd_wide_continuation;
  uint64_t size = 0;
  uint64_t width = 0;
  bool wr_clk_enable = false;
  std::string wr_clk_polarity;
  uint64_t wr_ports = 0;
  std::string wr_priority_mask;
  std::string wr_wide_continuation;

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    bool ift = false;
    if (params.contains("ift")) {
      ift = params["ift"].get<bool>();
    }

    Expects(params.contains("tag_size"));
    const uint32_t tag_size = params["tag_size"].get<uint32_t>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    auto read_data_signature = port_map.at("RD_DATA").m_bits_signature;
    auto read_address_signature = port_map.at("RD_ADDR").m_bits_signature;
    auto read_enable_signature = port_map.at("RD_EN").m_bits_signature;
    auto write_data_signature = port_map.at("WR_DATA").m_bits_signature;
    auto write_address_signature = port_map.at("WR_ADDR").m_bits_signature;
    auto write_clock_signature = port_map.at("WR_CLK").m_bits_signature;
    auto write_enable_signature = port_map.at("WR_EN").m_bits_signature;
    // instead of removing the '_' character from the memory name, we might want to remove the '\\' added by yosys to the memid before legalizing it?
    auto memory_name = legalize_identifier(memid);
    // in the mips_16_core_top design the memid is prefixed with a '_' character which is the reason the testbench cannot bind to the memory since it expects a memory name without the prefix
    if (memid.substr(0, 1) == "\\") {
      if (memory_name[0] == '_') {
        memory_name.replace(0, 1, "");
      }
    }
    std::stringstream result;

    result << fmt::format("reg [{}:0] {} [{}:0];\n", width - 1, memory_name, size - 1);

    result << fmt::format("initial begin\n");
    for (auto i = 0ull; i < size; ++i) {
      std::string_view view = std::string_view{init}.substr((size - i - 1) * width, width);
      result << fmt::format("  {} [{}] = {}'b{};\n", memory_name, i, width, view);
    }
    result << fmt::format("end\n");

    for (auto i = 0ull; i < rd_ports; ++i) {
      auto read_address_wire = fmt::format("{}_{}_{}", memory_name, legalize_identifier(identifiers_map.at(read_address_signature)), i);
      auto external_read_address_wire = abits == 1 ? legalize_identifier(identifiers_map.at(read_address_signature)) : fmt::format("{} [{}:{}]", legalize_identifier(identifiers_map.at(read_address_signature)), ((i + 1) * abits) - 1, i * abits);
      result << fmt::format("wire [{}:0] {} ;\n", abits - 1, read_address_wire);
      result << fmt::format("assign {} = {};\n", read_address_wire, external_read_address_wire);
      result << fmt::format("assign {} [{}:{}] = {} [{} ];\n", legalize_identifier(identifiers_map.at(read_data_signature)), ((i + 1) * width) - 1, i * width, memory_name, read_address_wire);
    }

    for (auto i = 0ull; i < wr_ports; ++i) {
      auto write_address_wire = fmt::format("{}_{}_{}", memory_name, legalize_identifier(identifiers_map.at(write_address_signature)), i);
      auto external_write_address_wire = abits == 1 ? legalize_identifier(identifiers_map.at(write_address_signature)) : fmt::format("{} [{}:{}]", legalize_identifier(identifiers_map.at(write_address_signature)), ((i + 1) * abits) - 1, i * abits);
      auto write_clock_name = wr_ports == 1 ? legalize_identifier(identifiers_map.at(write_clock_signature)) : fmt::format("{} [{}]", legalize_identifier(identifiers_map.at(write_clock_signature)), wr_ports - i - 1);
      result << fmt::format("wire [{}:0] {} ;\n", abits - 1, write_address_wire);
      result << fmt::format("assign {} = {};\n", write_address_wire, external_write_address_wire);
      result << fmt::format("always @({} {}) begin\n", "posedge", write_clock_name);
      for (auto j = 0ull; j < width; ++j) {
        result << fmt::format("  if ({} [{}])\n", legalize_identifier(identifiers_map.at(write_enable_signature)), (i * width) + j);
        result << fmt::format("    {} [{} ][{}:{}] <= {} [{}];\n", memory_name, write_address_wire, j, j, legalize_identifier(identifiers_map.at(write_data_signature)), (i * width) + j);
      }
      result << fmt::format("end\n");
    }

    if (ift) {
      result << fmt::format("reg [{}:0] {}{} [{}:0];\n", tag_size - 1, memory_name, ift_tag, size - 1);

      result << fmt::format("initial begin\n");
      for (auto i = 0ull; i < size; ++i) {
        result << fmt::format("  {}{} [{}] = 0;\n", memory_name, ift_tag, i);
      }
      result << fmt::format("end\n");

      std::vector<std::string> read_data_tag_wires;
      for (auto i = 0ull; i < rd_ports; ++i) {
        auto read_address_wire = fmt::format("{}_{}_{}", memory_name, legalize_identifier(identifiers_map.at(read_address_signature)), i);
        result << fmt::format("wire [{}:0] {}{} ;\n", tag_size - 1, read_address_wire, ift_tag);
        result << fmt::format("assign {}{} = {}{} [{} ] === {}'bX ? 0 : {}{} [{} ];\n", read_address_wire, ift_tag, memory_name, ift_tag, read_address_wire, tag_size, memory_name, ift_tag, read_address_wire);
        read_data_tag_wires.emplace_back(fmt::format("{}{}", read_address_wire, ift_tag));
      }

      auto read_data_tag_wires_or = boost::join(read_data_tag_wires, " | ");
      result << fmt::format("assign {}{} = {} | {}{} | {}{};\n", legalize_identifier(identifiers_map.at(read_data_signature)), ift_tag, read_data_tag_wires_or, legalize_identifier(identifiers_map.at(read_enable_signature)), ift_tag, legalize_identifier(identifiers_map.at(read_address_signature)), ift_tag);

      for (auto i = 0ull; i < wr_ports; ++i) {
        auto write_address_wire = fmt::format("{}_{}_{}", memory_name, legalize_identifier(identifiers_map.at(write_address_signature)), i);
        auto write_clock_name = wr_ports == 1 ? legalize_identifier(identifiers_map.at(write_clock_signature)) : fmt::format("{} [{}]", legalize_identifier(identifiers_map.at(write_clock_signature)), wr_ports - i - 1);
        result << fmt::format("always @({} {}) begin\n", "posedge", write_clock_name);
        result << fmt::format("  {}{} [{} ] <= {}{} | {}{} | {}{} ;\n", memory_name, ift_tag, write_address_wire, legalize_identifier(identifiers_map.at(write_data_signature)), ift_tag, legalize_identifier(identifiers_map.at(write_enable_signature)), ift_tag, legalize_identifier(identifiers_map.at(write_address_signature)), ift_tag);
        result << fmt::format("end\n");
      }
    }

    return result.str();
  }

  void export_smt2([[maybe_unused]] z3::context& ctx, [[maybe_unused]] z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {

    assert(!port_expr_map.empty());// size of port_expr_map unknown because this needs to include all the z3::expr for the memory element

    // Ensure the memory register name is provided in the parameters
    assert(params.contains("mem_reg_name"));
    std::string mem_reg_name = params["mem_reg_name"];
    std::string mem_reg_name_previous = params.contains("mem_reg_name_previous") ? params["mem_reg_name_previous"] : "";
    assert(!mem_reg_name.empty());
    assert(!mem_reg_name_previous.empty());

    // Define the bitvector sorts for Address and Data
    unsigned addr_size = port_expr_map.at("RD_ADDR").get_sort().bv_size();
    unsigned data_size = port_expr_map.at("RD_DATA").get_sort().bv_size();

    // Define the memory as an array of Data indexed by Address
    z3::sort bv_addr = ctx.bv_sort(addr_size);
    z3::sort bv_data = ctx.bv_sort(data_size);
    z3::sort memory_sort = ctx.array_sort(bv_addr, bv_data);

    // Create the memory expression
    const z3::expr& mem = port_expr_map.at(mem_reg_name);
    const z3::expr& mem_previous = params.contains("mem_reg_name_previous") ? port_expr_map.at(mem_reg_name_previous) : port_expr_map.at(mem_reg_name);

    // Create the read enable, read address, and read data expressions
    z3::expr rd_en = port_expr_map.at("RD_EN");
    z3::expr rd_addr = port_expr_map.at("RD_ADDR");
    z3::expr rd_data = port_expr_map.at("RD_DATA");

    // Create the condition for the ITE (if-then-else) expression
    z3::expr rd_condition = (rd_en == ctx.bv_val(1, rd_en.get_sort().bv_size()));

    // Assuming `mem` is the Z3 memory expression
    z3::sort mem_sort = mem.get_sort();

    // Check if it's an array sort (memory is typically represented as an array)
    if (mem_sort.is_array()) {
      z3::sort domain_sort = mem_sort.array_domain();// Address size
      z3::sort range_sort = mem_sort.array_range();  // Data size

      spdlog::info("Memory domain (address size): {}", domain_sort.bv_size());
      spdlog::info("Memory range (data size): {}", range_sort.bv_size());
    } else {
      spdlog::info("The mem expression is not an array.");
    }

    // Create the ITE expression: if (RD_EN == 1) then select(mem, RD_ADDR) else x
    z3::expr rd_ite_expr = z3::ite(rd_condition, z3::select(mem, rd_addr), ctx.bv_const("x", data_size));

    // Add the read assertion to the solver
    solver.add(rd_data == rd_ite_expr);


    if (port_expr_map.contains("WR_EN")) {
      // Create the write enable, write address, and write data expressions
      z3::expr wr_en = port_expr_map.at("WR_EN");
      z3::expr wr_addr = port_expr_map.at("WR_ADDR");
      z3::expr wr_data = port_expr_map.at("WR_DATA");

      const int32_t max_write_enable_value = 65535;
      // Create the condition for the write operation
      z3::expr wr_condition = (wr_en == ctx.bv_val(max_write_enable_value, wr_en.get_sort().bv_size()));

      // Create the updated memory expression: if (WR_EN == 1) then store(mem_previous, WR_ADDR, WR_DATA) else mem_previous
      z3::expr updated_mem = z3::ite(wr_condition, z3::store(mem_previous, wr_addr, wr_data), mem_previous);

      // Assert that all other indices remain unchanged symbolically
      z3::expr forall_condition = z3::forall(
          ctx.bv_const("i", addr_size),// Symbolic index
          z3::implies(
              ctx.bv_const("i", addr_size) != wr_addr,                                                                       // If the index is not the write address
              z3::select(updated_mem, ctx.bv_const("i", addr_size)) == z3::select(mem_previous, ctx.bv_const("i", addr_size))// Values must remain the same
              ));

      // Add the write assertion to the solver
      solver.add(forall_condition);

      // Add the write assertion to the solver
      solver.add(mem == updated_mem);
    }
  }
};
}// namespace ducode
