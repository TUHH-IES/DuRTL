/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

#include <gsl/assert>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdint>

namespace ducode {

class CellPMux : public Cell {
public:
  uint64_t width;
  uint64_t s_width;
  bool full_case = false;

  CellPMux(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {

    Expects(ports.size() == 4);
    width = std::stoull(parameters.at("WIDTH").get<std::string>(), nullptr, 2);
    s_width = std::stoull(parameters.at("S_WIDTH").get<std::string>(), nullptr, 2);
    if (attributes.contains("full_case")) {
      full_case = std::stoull(attributes.at("full_case").get<std::string>(), nullptr, 2) != 0;
    }
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(params.contains("ift"));
    const bool ift = params["ift"].get<bool>();

    Expects(params.contains("tag_size"));
    const uint32_t tag_size = params["tag_size"].get<uint32_t>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;

    // strings for the ports
    std::string a;
    std::string b;
    std::string s;
    std::string y;
    uint64_t b_width = 0;

    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        assert(port.m_name == "Y");
        y = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "A") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        a = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "B") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        b = identifiers_map.at(port.m_bits_signature);
        b_width = port.m_bits.size();
      }
      if (port.m_name == "S") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        s = identifiers_map.at(port.m_bits_signature);
      }
    }

    std::string case_vec = std::string(s_width - 1, '?') + "1";
    result << fmt::format("function [{}:0] {} ;\n", width - 1, legalize_identifier(m_name));
    result << fmt::format("  input [{}:0] a;\n", width - 1);
    result << fmt::format("  input [{}:0] b;\n", b_width - 1);
    result << fmt::format("  input [{}:0] s;\n", s_width - 1);
    result << fmt::format("  casez (s) {}\n", full_case ? "// synopsys parallel_case" : "");
    for (auto i = 0ull; i < s_width; i++) {
      result << fmt::format("    {}'b{} : begin \n", s_width, case_vec);
      result << fmt::format("      {} = b[{}:{}];\n", legalize_identifier(m_name), ((i + 1) * width) - 1, i * width);
      result << "   end\n";
      std::ranges::rotate(case_vec, case_vec.begin() + 1);
    }
    result << fmt::format("    default: begin\n");
    result << fmt::format("      {} = a;\n", legalize_identifier(m_name));
    result << "   end\n  endcase\n endfunction\n";
    result << fmt::format("assign {} = {} ({}, {}, {});\n", y, legalize_identifier(m_name), a, b, s);

    if (ift) {
      result << fmt::format("function [{}:0] {}{} ;\n", tag_size - 1, legalize_identifier(m_name), ift_tag);
      result << fmt::format("  input [{}:0] a;\n", tag_size - 1);
      result << fmt::format("  input [{}:0] b;\n", tag_size - 1);
      result << fmt::format("  input [{}:0] s;\n", s_width - 1);
      result << fmt::format("  casez (s) {}\n", full_case ? "// synopsys parallel_case" : "");
      for (auto i = 0ull; i < s_width; i++) {
        result << fmt::format("    {}'b{} : begin \n", s_width, case_vec);
        result << fmt::format("      {}{} = b;", legalize_identifier(m_name), ift_tag);
        result << "   end\n";
        std::ranges::rotate(case_vec, case_vec.begin() + 1);
      }
      result << fmt::format("    default: begin\n");
      result << fmt::format("      {}{} = a;", legalize_identifier(m_name), ift_tag);
      result << "   end\n  endcase\n endfunction\n";
      result << fmt::format("assign {}{} = {}{} ({}{}, {}{}, {} ) | {}{};\n", y, ift_tag, legalize_identifier(m_name), ift_tag, a, ift_tag, b, ift_tag, s, s, ift_tag);
    }

    return result.str();
  }

  void export_smt2(z3::context& ctx, z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {

    assert(port_expr_map.size() <= 4);

    uint32_t select_size = port_expr_map.at("S").get_sort().bv_size();
    uint32_t output_size = port_expr_map.at("Y").get_sort().bv_size();
    uint32_t a_size = port_expr_map.at("A").get_sort().bv_size();
    uint32_t b_size = port_expr_map.at("B").get_sort().bv_size();

    spdlog::debug("a_size: {} b_size: {} select_size: {} output_size: {}", a_size, b_size, select_size, output_size);

    z3::expr_vector results(ctx);

    results.push_back(to_expr(ctx, port_expr_map.at("B").extract((select_size * output_size) - 1, (select_size - 1) * output_size)));

    for (uint32_t i = select_size - 1; i > 0; i--) {
      results.push_back(to_expr(ctx, Z3_mk_ite(ctx, (port_expr_map.at("S").extract(i - 1, i - 1) == 1), port_expr_map.at("B").extract(((i) *output_size) - 1, (i - 1) * output_size), results[(select_size - i) - 1])));
    }
    solver.add((port_expr_map.at("Y") == to_expr(ctx, Z3_mk_ite(ctx, (port_expr_map.at("S") == 0), port_expr_map.at("A"), results.back()))));
  }
};
}// namespace ducode
