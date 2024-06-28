/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

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

    auto memory_name = legalize_identifier(memid);

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
      auto external_read_address_wire = abits == 1 ? legalize_identifier(identifiers_map.at(read_address_signature)) : fmt::format("{} [{}:{}]", legalize_identifier(identifiers_map.at(read_address_signature)), (i + 1) * abits - 1, i * abits);
      result << fmt::format("wire [{}:0] {} ;\n", abits - 1, read_address_wire);
      result << fmt::format("assign {} = {};\n", read_address_wire, external_read_address_wire);
      result << fmt::format("assign {} [{}:{}] = {} [{} ];\n", legalize_identifier(identifiers_map.at(read_data_signature)), (i + 1) * width - 1, i * width, memory_name, read_address_wire);
    }

    for (auto i = 0ull; i < wr_ports; ++i) {
      auto write_address_wire = fmt::format("{}_{}_{}", memory_name, legalize_identifier(identifiers_map.at(write_address_signature)), i);
      auto external_write_address_wire = abits == 1 ? legalize_identifier(identifiers_map.at(write_address_signature)) : fmt::format("{} [{}:{}]", legalize_identifier(identifiers_map.at(write_address_signature)), (i + 1) * abits - 1, i * abits);
      auto write_clock_name = wr_ports == 1 ? legalize_identifier(identifiers_map.at(write_clock_signature)) : fmt::format("{} [{}]", legalize_identifier(identifiers_map.at(write_clock_signature)), wr_ports - i - 1);
      result << fmt::format("wire [{}:0] {} ;\n", abits - 1, write_address_wire);
      result << fmt::format("assign {} = {};\n", write_address_wire, external_write_address_wire);
      result << fmt::format("always @({} {}) begin\n", "posedge", write_clock_name);
      for (auto j = 0ull; j < width; ++j) {
        result << fmt::format("  if ({} [{}])\n", legalize_identifier(identifiers_map.at(write_enable_signature)), i * width + j);
        result << fmt::format("    {} [{} ][{}:{}] <= {} [{}];\n", memory_name, write_address_wire, j, j, legalize_identifier(identifiers_map.at(write_data_signature)), i * width + j);
      }
      result << fmt::format("end\n");
    }

    if (ift) {
      result << fmt::format("reg [{}:0] {}_t [{}:0];\n", tag_size - 1, memory_name, size - 1);

      result << fmt::format("initial begin\n");
      for (auto i = 0ull; i < size; ++i) {
        result << fmt::format("  {}_t [{}] = 0;\n", memory_name, i);
      }
      result << fmt::format("end\n");

      std::vector<std::string> read_data_tag_wires;
      for (auto i = 0ull; i < rd_ports; ++i) {
        auto read_address_wire = fmt::format("{}_{}_{}", memory_name, legalize_identifier(identifiers_map.at(read_address_signature)), i);
        result << fmt::format("wire [{}:0] {}_t ;\n", tag_size - 1, read_address_wire);
        result << fmt::format("assign {}_t = {}_t [{} ] === {}'bX ? 0 : {}_t [{} ];\n", read_address_wire, memory_name, read_address_wire, tag_size, memory_name, read_address_wire);
        read_data_tag_wires.emplace_back(fmt::format("{}_t", read_address_wire));
      }

      auto read_data_tag_wires_or = boost::join(read_data_tag_wires, " | ");
      result << fmt::format("assign {}_t = {} | {}_t | {}_t;\n", legalize_identifier(identifiers_map.at(read_data_signature)), read_data_tag_wires_or, legalize_identifier(identifiers_map.at(read_enable_signature)), legalize_identifier(identifiers_map.at(read_address_signature)));

      for (auto i = 0ull; i < wr_ports; ++i) {
        auto write_address_wire = fmt::format("{}_{}_{}", memory_name, legalize_identifier(identifiers_map.at(write_address_signature)), i);
        auto write_clock_name = wr_ports == 1 ? legalize_identifier(identifiers_map.at(write_clock_signature)) : fmt::format("{} [{}]", legalize_identifier(identifiers_map.at(write_clock_signature)), wr_ports - i - 1);
        result << fmt::format("always @({} {}) begin\n", "posedge", write_clock_name);
        result << fmt::format("  {}_t [{} ] <= {}_t | {}_t | {}_t ;\n", memory_name, write_address_wire, legalize_identifier(identifiers_map.at(write_data_signature)), legalize_identifier(identifiers_map.at(write_enable_signature)), legalize_identifier(identifiers_map.at(write_address_signature)));
        result << fmt::format("end\n");
      }
    }

    return result.str();
  }
};
}// namespace ducode