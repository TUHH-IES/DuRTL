/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/sim.hpp>
#include <ducode/testbench.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

/*
 * Contains all experiments for the approximate design analysis paper
 */

// counts the number of tags on a single given edge - used here for specific output edges
inline uint32_t tag_cnt_single_edge(ducode::DesignInstance& instance, std::string& edge_name) {
  uint32_t tag_cnt = 0;
  std::set<uint32_t> tags;
  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == edge_name)) {
      //        auto tag_values = instance.get_tag_values(*edge_it);
      std::vector<const VCDSignalValues*> result;
      for (const auto& [vcd_index, vcd_data]: ranges::views::enumerate(instance.m_vcd_data)) {
        tags.clear();
        result.clear();
        // some edges do not have simulation values assigned to them
        if (!instance.m_graph[*edge_it].tags_signal.empty()) {
          result.emplace_back(&(vcd_data->get_signal_values(instance.m_graph[*edge_it].tags_signal[vcd_index]->hash)));
        }
        for (const auto* tag_vector: result) {
          for (const auto& tag_value: *tag_vector) {
            auto value_vector = tag_value.value.get_value_vector();
            for (const auto& [index, tag_bit]: ranges::views::enumerate(value_vector)) {
              if (tag_bit == VCDBit::VCD_1) {
                if (!tags.contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))))) {
                  tags.insert(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))));
                  tag_cnt++;
                }
              }
            }
          }
        }
      }
      break;
    }
    //    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_2")) {
  }
  return tag_cnt;
}

// counts the total number of tags across all given edges - used here for all output edges
inline uint32_t tag_cnt_multiple_edges(ducode::DesignInstance& instance, std::vector<std::string>& edge_names) {
  uint32_t tag_cnt = 0;
  std::set<uint32_t> tags;
  std::vector<std::set<uint32_t>> total_tags;
  for (uint32_t i = 0; i < instance.m_vcd_data.size(); i++) {
    total_tags.emplace_back(tags);
  }
  for (auto& edge_name: edge_names) {
    for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
      if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == edge_name)) {
        //        auto tag_values = instance.get_tag_values(*edge_it);
        std::vector<const VCDSignalValues*> result;
        for (const auto& [vcd_index, vcd_data]: ranges::views::enumerate(instance.m_vcd_data)) {
          tags.clear();
          result.clear();
          // some edges do not have simulation values assigned to them
          if (!instance.m_graph[*edge_it].tags_signal.empty()) {
            result.emplace_back(&(vcd_data->get_signal_values(instance.m_graph[*edge_it].tags_signal[vcd_index]->hash)));
          }
          for (const auto* tag_vector: result) {
            for (const auto& tag_value: *tag_vector) {
              auto value_vector = tag_value.value.get_value_vector();
              for (const auto& [index, tag_bit]: ranges::views::enumerate(value_vector)) {
                if (tag_bit == VCDBit::VCD_1) {
                  if (!tags.contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1))))) && !total_tags[vcd_index].contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))))) {
                    tags.insert(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))));
                    tag_cnt++;
                  }
                }
              }
            }
          }
          total_tags[vcd_index].merge(tags);
        }
        break;
      }
      //    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_2")) {
    }
  }
  return tag_cnt;
}

inline uint32_t unique_tag_cnt_single_edge(ducode::DesignInstance& instance, std::string& edge_name) {
  uint32_t tag_cnt = 0;
  std::set<uint32_t> tags;
  std::vector<std::set<uint32_t>> tags_ci_inputs;
  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_2")) {
      //        auto tag_values = instance.get_tag_values(*edge_it);
      std::vector<const VCDSignalValues*> result;
      for (const auto& [vcd_index, vcd_data]: ranges::views::enumerate(instance.m_vcd_data)) {
        //        tags.clear();
        result.clear();
        // some edges do not have simulation values assigned to them
        if (!instance.m_graph[*edge_it].tags_signal.empty()) {
          result.emplace_back(&(vcd_data->get_signal_values(instance.m_graph[*edge_it].tags_signal[vcd_index]->hash)));
        }
        for (const auto* tag_vector: result) {
          for (const auto& tag_value: *tag_vector) {
            auto value_vector = tag_value.value.get_value_vector();
            for (const auto& [index, tag_bit]: ranges::views::enumerate(value_vector)) {
              if (tag_bit == VCDBit::VCD_1) {
                if (!tags.contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))))) {
                  tags.insert(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))));
                }
              }
            }
          }
        }
        tags_ci_inputs.emplace_back(tags);
      }
    }
    //    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_2")) {
  }
  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == edge_name)) {
      //        auto tag_values = instance.get_tag_values(*edge_it);
      std::vector<const VCDSignalValues*> result;
      for (const auto& [vcd_index, vcd_data]: ranges::views::enumerate(instance.m_vcd_data)) {
        tags.clear();
        result.clear();
        // some edges do not have simulation values assigned to them
        if (!instance.m_graph[*edge_it].tags_signal.empty()) {
          result.emplace_back(&(vcd_data->get_signal_values(instance.m_graph[*edge_it].tags_signal[vcd_index]->hash)));
        }
        for (const auto* tag_vector: result) {
          for (const auto& tag_value: *tag_vector) {
            auto value_vector = tag_value.value.get_value_vector();
            for (const auto& [index, tag_bit]: ranges::views::enumerate(value_vector)) {
              if (tag_bit == VCDBit::VCD_1) {
                if (!tags.contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1))))) && tags_ci_inputs[vcd_index].contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))))) {
                  tags.insert(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))));
                  tag_cnt++;
                }
              }
            }
          }
        }
      }
      break;
    }
    //    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_2")) {
  }
  return tag_cnt;
}

inline uint32_t unique_tag_cnt_from_input_to_multiple_edges(ducode::DesignInstance& instance, std::string& input_name, std::string& output_name) {
  uint32_t tag_cnt = 0;
  std::set<uint32_t> tags;
  std::vector<std::set<uint32_t>> tags_ci_inputs;
  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == input_name)) {
      //        auto tag_values = instance.get_tag_values(*edge_it);
      std::vector<const VCDSignalValues*> result;
      for (const auto& [vcd_index, vcd_data]: ranges::views::enumerate(instance.m_vcd_data)) {
        tags.clear();
        result.clear();
        // some edges do not have simulation values assigned to them
        if (!instance.m_graph[*edge_it].tags_signal.empty()) {
          result.emplace_back(&(vcd_data->get_signal_values(instance.m_graph[*edge_it].tags_signal[vcd_index]->hash)));
        }
        for (const auto* tag_vector: result) {
          for (const auto& tag_value: *tag_vector) {
            auto value_vector = tag_value.value.get_value_vector();
            for (const auto& [index, tag_bit]: ranges::views::enumerate(value_vector)) {
              if (tag_bit == VCDBit::VCD_1) {
                if (!tags.contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))))) {
                  tags.insert(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))));
                }
              }
            }
          }
        }
        tags_ci_inputs.emplace_back(tags);
      }
      break;
    }
    //    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_2")) {
  }
  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == output_name)) {
      //        auto tag_values = instance.get_tag_values(*edge_it);
      std::vector<const VCDSignalValues*> result;
      for (const auto& [vcd_index, vcd_data]: ranges::views::enumerate(instance.m_vcd_data)) {
        tags.clear();
        result.clear();
        // some edges do not have simulation values assigned to them
        if (!instance.m_graph[*edge_it].tags_signal.empty()) {
          result.emplace_back(&(vcd_data->get_signal_values(instance.m_graph[*edge_it].tags_signal[vcd_index]->hash)));
        }
        for (const auto* tag_vector: result) {
          for (const auto& tag_value: *tag_vector) {
            auto value_vector = tag_value.value.get_value_vector();
            for (const auto& [index, tag_bit]: ranges::views::enumerate(value_vector)) {
              if (tag_bit == VCDBit::VCD_1) {
                if (!tags.contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1))))) && tags_ci_inputs[vcd_index].contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))))) {
                  tags.insert(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))));
                  tag_cnt++;
                }
              }
            }
          }
        }
      }
      break;
    }
    //    if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub1_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_add2_2" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_1" || instance.m_graph[*edge_it].net_ptr->get_name() == "ci_sub2_2")) {
  }
  return tag_cnt;
}

void write_heat_map(std::string tex_name, std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_map, std::vector<std::string>& i_names, std::vector<std::string> o_names) {
  double largest_number = 0;
  for (auto& tag_cnt: tag_cnt_map) {
    for (auto& tags: tag_cnt.second) {
      largest_number = std::max<double>(tags.second, largest_number);
    }
  }

  std::ofstream ofv_tb(tex_name);
  if (!ofv_tb.is_open()) {
    throw std::runtime_error(fmt::format("Could not open file: {}", tex_name));
  }
  ofv_tb << "\\begin{table*}\n";
  ofv_tb << "\\caption{Appx. Adder heat map}\n";
  ofv_tb << "\\label{tab:heat_map_random_rel}\n";
  ofv_tb << "\\begin{tabularx}{\\textwidth}{Xcccccccccc}\n";
  ofv_tb << "\\toprule\n";
  ofv_tb << "  & ";
  for (auto& name: o_names) {
    std::string nam = name;
    if (name.find("_") != std::string::npos) {
      const int num1 = 2;
      const int num2 = 8;
      nam.replace(num1, 1, "\\_");
      nam.replace(num2, 1, "\\_");
    }
    ofv_tb << nam;
    if (name != o_names.back()) {
      ofv_tb << "&";
    } else {
      ofv_tb << "  \\\\ \n";
    }
  }
  ofv_tb << "\\midrule\n";
  for (auto& name: i_names) {
    std::string nam = name;
    if (name.find("_") != std::string::npos) {
      const int num1 = 2;
      const int num2 = 8;
      nam.replace(num1, 1, "\\_");
      nam.replace(num2, 1, "\\_");
    }
    ofv_tb << nam << "&";
    std::vector<uint32_t> color_gradient = {0, 0, 0};
    for (auto& o_name: o_names) {
      const double num1 = 0.25;
      const double num2 = 0.50;
      const double num3 = 0.75;
      if (tag_cnt_map[o_name][name] == 0) {
        ofv_tb << "\\cellcolor{white}";
      } else if ((tag_cnt_map[o_name][name] / largest_number) <= num1) {
        ofv_tb << "\\cellcolor{green}";
      } else if ((tag_cnt_map[o_name][name] / largest_number) > num1 && (tag_cnt_map[o_name][name] / largest_number) <= num2) {
        ofv_tb << "\\cellcolor{yellow}";
      } else if ((tag_cnt_map[o_name][name] / largest_number) > num2 && (tag_cnt_map[o_name][name] / largest_number) <= num3) {
        ofv_tb << "\\cellcolor{orange}";
      }
      if ((tag_cnt_map[o_name][name] / largest_number) > num3) {
        ofv_tb << "\\cellcolor{red}";
      }
      //      if (tag_cnt_map[o_name][name] == 0) {
      //        ofv_tb << "\\cellcolor[RGB]{255,255,255}";
      //      } else {
      //        double t = (tag_cnt_map[o_name][name] / largest_number);
      //        color_gradient[0] = (1-t) * 255;
      //        color_gradient[1] = (1-t) * 255;
      //        color_gradient[2] = t * 255;
      //        color_gradient[0] = 255 * (0.5 * sin(3.14 * (t + 0.5)) + 0.5);
      //        color_gradient[1] = 255 * sin(1.57 * t);
      //        color_gradient[2] = 255 * cos(1.57 * t);

      //        ofv_tb << fmt::format("\\cellcolor[RGB]{{ {},{},{} }}", color_gradient[0], color_gradient[1], color_gradient[2]);
      //      }
      ofv_tb << tag_cnt_map[o_name][name];
      if (o_name != o_names.back()) {
        ofv_tb << "&";
      }
    }
    ofv_tb << "\\\\ \n";
  }

  ofv_tb << "\\end{tabularx}\n";
  ofv_tb << "\\end{table*}\n";


  ofv_tb.close();
};

TEST_CASE("appx_add", "[exper_apprx][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = true;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_appx_add" / "fft16_apx.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_appx_add" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  //  auto tag_map = generate_tags_full_resolution(tb_module, vcd_data->get_timestamps().back(), stepsize);
  const int tag_number = 0;
  auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tag_number, tb_module->get_ports());

  //  auto tag_map = generate_tags_random(tb_module, vcd_data->get_timestamps().back(), stepsize, 0);

  design.write_verilog(exported_verilog, params);

  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

  bool contains_x = false;

  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr) {
      auto tag_values = instance.get_tag_values(*edge_it);
      for (const auto* tag_vector: tag_values) {
        for (const auto& tag_value: *tag_vector) {
          for (const auto& tag_bit: tag_value.value.get_value_vector()) {
            if (tag_bit == VCDBit::VCD_X) {
              contains_x = true;
              break;
            }
          }
        }
      }
    }
  }

  CHECK_FALSE(contains_x);
}
TEST_CASE("appx_add_experiments", "[exper_apprx][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = true;
  nlohmann::json result_data;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "apprx_add_experiments" / "fft16_apx.json";
  //  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "appx_add_experiments" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  //  auto tag_map = generate_tags_full_resolution(tb_module, vcd_data->get_timestamps().back(), stepsize);

  design.write_verilog(exported_verilog, params);
  const int max_num = 6;
  for (uint32_t i = 0; i < max_num; i++) {
    std::string vcd_file = "tb_fft16_ofdm_apx_";
    vcd_file += std::to_string(i) + ".vcd";
    auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "apprx_add_experiments" / vcd_file;
    VCDFileParser vcd_obj;
    auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());
    //    auto tag_map = generate_tags_random(tb_module, vcd_data->get_timestamps().back(), stepsize, 0);
    const int tag_number = 0;
    auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tag_number, tb_module->get_ports());

    auto simulation_runtime_start = std::chrono::high_resolution_clock ::now();

    ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

    auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
    result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(simulation_runtime_end - simulation_runtime_start).count();

    std::set<uint32_t> tags;
    for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
      if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "qi" || instance.m_graph[*edge_it].net_ptr->get_name() == "qr")) {
        auto tag_values = instance.get_tag_values(*edge_it);
        for (const auto* tag_vector: tag_values) {
          for (const auto& tag_value: *tag_vector) {
            auto value_vector = tag_value.value.get_value_vector();
            for (const auto& [index, tag_bit]: ranges::views::enumerate(value_vector)) {
              if (tag_bit == VCDBit::VCD_1) {
                if (!tags.contains(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))))) {
                  tags.insert(static_cast<unsigned int>(pow(2, (static_cast<double>(tag_value.value.get_value_vector().size() - index - 1)))));
                }
              }
            }
          }
        }
      }
    }
    result_data["injected_tags_@_outputs" + vcd_file] = tags.size();// remove the 0 entry; not a tag
  }
  std::fstream os_json("results.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

////// TEST CASES FOR THE EXPERIMENTS FOR THE PAPER "Detailed Insight into Approximate Circuits with Error Responsive Information Flow Tracking"
TEST_CASE("fft16_full_res", "[exper_exact][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  params["apprx_mult"] = false;
  nlohmann::json result_data;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft16" / "fft16.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft16" / "tb_fft16_v2.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  std::cout << temp_dir << '\n';
  ducode::Design design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tb_module->get_ports());
  //  auto tag_map = generate_tags_random(tb_module, vcd_data->get_timestamps().back(), stepsize, 1000);

  design.write_verilog(exported_verilog, params);

  std::string vcd_file = "tb_fft16_ofdm_";
  std::string heat_map_file = vcd_file + "_hmap_full_res";
  vcd_file += std::to_string(0) + ".vcd";

  auto simulation_runtime_start = std::chrono::high_resolution_clock ::now();

  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

  auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
  result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(simulation_runtime_end - simulation_runtime_start).count();

  uint32_t tag_cnt_outputs_total = 0;
  uint32_t tag_cnt_outputs = 0;
  uint32_t tag_cnt_ci_inputs = 0;
  std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
  std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
  std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
  std::vector<std::string> inputs_names = {"clk", "rstn", "ena", "xr", "xi"};
  std::vector<std::string> outputs_names = {"qr", "qi"};

  tag_cnt_outputs_total = tag_cnt_multiple_edges(instance, outputs_names);

  for (auto& name: inputs_names) {
    tag_cnt_inputs[name] = tag_cnt_single_edge(instance, name);
  }
  for (auto& o_name: outputs_names) {
    tag_cnt_output_1_input.clear();
    for (auto& i_name: inputs_names) {
      tag_cnt_output_1_input[i_name] = unique_tag_cnt_from_input_to_multiple_edges(instance, i_name, o_name);
    }
    tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
  }

  write_heat_map((heat_map_file + std::to_string(0) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names, outputs_names);

  result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  std::fstream os_json("results_fullres.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

TEST_CASE("fft16_random_res", "[exper_exact][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  params["apprx_mult"] = false;
  nlohmann::json result_data;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft16" / "fft16.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft16" / "tb_fft16_v2.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  std::cout << temp_dir << '\n';
  ducode::Design design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  const int tag_number = 500;
  auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tag_number, tb_module->get_ports());

  design.write_verilog(exported_verilog, params);

  std::string vcd_file = "tb_fft16_ofdm";
  std::string heat_map_file = vcd_file + "_random_res";
  vcd_file += std::to_string(0) + ".vcd";

  auto simulation_runtime_start = std::chrono::high_resolution_clock ::now();

  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

  auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
  result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(simulation_runtime_end - simulation_runtime_start).count();

  uint32_t tag_cnt_outputs_total = 0;
  uint32_t tag_cnt_outputs = 0;
  uint32_t tag_cnt_ci_inputs = 0;
  std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
  std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
  std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
  std::vector<std::string> inputs_names = {"clk", "rstn", "ena", "xr", "xi"};
  std::vector<std::string> outputs_names = {"qr", "qi"};

  tag_cnt_outputs_total = tag_cnt_multiple_edges(instance, outputs_names);

  for (auto& name: inputs_names) {
    tag_cnt_inputs[name] = tag_cnt_single_edge(instance, name);
  }
  for (auto& o_name: outputs_names) {
    tag_cnt_output_1_input.clear();
    for (auto& i_name: inputs_names) {
      tag_cnt_output_1_input[i_name] = unique_tag_cnt_from_input_to_multiple_edges(instance, i_name, o_name);
    }
    tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
  }

  write_heat_map((heat_map_file + std::to_string(0) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names, outputs_names);

  result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  std::fstream os_json("results_randomres.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

TEST_CASE("appx_add_experiments_larger_WI", "[exper_apprx_wi][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = true;
  nlohmann::json result_data;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "apprx_add_experiments_WI" / "fft16_apx.json";
  //  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "appx_add_experiments" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  //  auto tag_map = generate_tags_full_resolution(tb_module, vcd_data->get_timestamps().back(), stepsize);

  design.write_verilog(exported_verilog, params);
  for (uint32_t i = 3; i < 4; i++) {
    std::string vcd_file = "tb_fft16_ofdm_apx_";
    std::string heat_map_file = vcd_file + "_hmap_random_res";

    vcd_file += std::to_string(i) + ".vcd";
    auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "apprx_add_experiments_WI" / vcd_file;
    VCDFileParser vcd_obj;
    auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());
    //    auto tag_map = generate_tags_random(tb_module, vcd_data->get_timestamps().back(), stepsize, 0);
    const int tag_number = 0;
    auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tag_number, tb_module->get_ports());

    auto simulation_runtime_start = std::chrono::high_resolution_clock ::now();

    std::unordered_map<std::string, std::string> tagged_ports = {{"ci_add1_1", "co_add1_1"}, {"ci_add1_2", "co_add1_2"}, {"ci_add2_1", "co_add2_1"}, {"ci_add2_2", "co_add2_2"}, {"ci_sub1_1", "co_sub1_1"}, {"ci_sub1_2", "co_sub1_2"}, {"ci_sub2_1", "co_sub2_1"}, {"ci_sub2_2", "co_sub2_2"}};
    ducode::DesignInstance instance = ducode::do_simulation(design, vcd_data, exported_verilog, exported_testbench, tagged_ports, params);

    //    ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

    auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
    result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(simulation_runtime_end - simulation_runtime_start).count();
    uint32_t tag_cnt_outputs_total = 0;
    uint32_t tag_cnt_outputs = 0;
    uint32_t tag_cnt_ci_inputs = 0;
    std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
    std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
    std::vector<std::string> inputs_names = {"ci_add1_1", "ci_add1_2", "ci_sub1_1", "ci_sub1_2", "ci_add2_1", "ci_add2_2", "ci_sub2_1", "ci_sub2_2", "clk", "rstn", "ena", "xr", "xi"};
    std::vector<std::string> outputs_names = {"co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2", "qr", "qi"};
    std::vector<std::string> ci_outputs_names = {"co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2"};

    tag_cnt_outputs_total = tag_cnt_multiple_edges(instance, outputs_names);
    tag_cnt_outputs = tag_cnt_multiple_edges(instance, ci_outputs_names);
    const int index0 = 0;
    const int index1 = 1;
    const int index2 = 2;
    const int index3 = 3;
    const int index4 = 4;
    const int index5 = 5;
    const int index6 = 6;
    const int index7 = 7;
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index0]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index1]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index2]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index3]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index4]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index5]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index6]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index7]);
    for (auto& name: inputs_names) {
      tag_cnt_inputs[name] = tag_cnt_single_edge(instance, name);
    }
    for (auto& o_name: outputs_names) {
      tag_cnt_output_1_input.clear();
      for (auto& i_name: inputs_names) {
        tag_cnt_output_1_input[i_name] = unique_tag_cnt_from_input_to_multiple_edges(instance, i_name, o_name);
      }
      tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
    }

    std::vector<std::string> inputs_names_heat_map = {"ci_add1_1", "ci_add1_2", "ci_sub1_1", "ci_sub1_2", "ci_add2_1", "ci_add2_2", "ci_sub2_1", "ci_sub2_2"};

    write_heat_map((heat_map_file + std::to_string(i) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names_heat_map, outputs_names);

    result_data["injected_tags_@_ci_outputs_" + vcd_file] = tag_cnt_outputs;
    result_data["injected_tags_@_ci_inputs_" + vcd_file] = tag_cnt_ci_inputs;
    result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  }
  std::fstream os_json("results.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

TEST_CASE("appx_add_experiments_full_res", "[exper_apprx_wi][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  nlohmann::json result_data;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "apprx_add_experiments_WI" / "fft16_apx.json";
  //  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "appx_add_experiments" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  //  auto tag_map = generate_tags_full_resolution(tb_module, vcd_data->get_timestamps().back(), stepsize);

  design.write_verilog(exported_verilog, params);
  for (uint32_t i = 3; i < 4; i++) {
    std::string vcd_file = "tb_fft16_ofdm_apx_";
    std::string heat_map_file = vcd_file + "_hmap_full_res";
    vcd_file += std::to_string(i) + ".vcd";
    auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "apprx_add_experiments" / vcd_file;
    VCDFileParser vcd_obj;
    auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());
    auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tb_module->get_ports());

    auto simulation_runtime_start = std::chrono::high_resolution_clock ::now();

    ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

    auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
    result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(simulation_runtime_end - simulation_runtime_start).count();

    uint32_t tag_cnt_outputs_total = 0;
    uint32_t tag_cnt_outputs = 0;
    uint32_t tag_cnt_ci_inputs = 0;
    std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
    std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
    std::vector<std::string> inputs_names = {"ci_add1_1", "ci_add1_2", "ci_sub1_1", "ci_sub1_2", "ci_add2_1", "ci_add2_2", "ci_sub2_1", "ci_sub2_2", "clk", "rstn", "ena", "xr", "xi"};
    std::vector<std::string> outputs_names = {"co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2", "qr", "qi"};
    std::vector<std::string> ci_outputs_names = {"co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2"};

    tag_cnt_outputs_total = tag_cnt_multiple_edges(instance, outputs_names);
    tag_cnt_outputs = tag_cnt_multiple_edges(instance, ci_outputs_names);
    const int index0 = 0;
    const int index1 = 1;
    const int index2 = 2;
    const int index3 = 3;
    const int index4 = 4;
    const int index5 = 5;
    const int index6 = 6;
    const int index7 = 7;
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index0]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index1]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index2]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index3]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index4]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index5]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index6]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index7]);
    for (auto& name: inputs_names) {
      tag_cnt_inputs[name] = tag_cnt_single_edge(instance, name);
    }
    for (auto& o_name: outputs_names) {
      tag_cnt_output_1_input.clear();
      for (auto& i_name: inputs_names) {
        tag_cnt_output_1_input[i_name] = unique_tag_cnt_from_input_to_multiple_edges(instance, i_name, o_name);
      }
      tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
    }

    write_heat_map((heat_map_file + std::to_string(i) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names, outputs_names);

    result_data["injected_tags_@_ci_outputs_" + vcd_file] = tag_cnt_outputs;
    result_data["injected_tags_@_ci_inputs_" + vcd_file] = tag_cnt_ci_inputs;
    result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  }
  std::fstream os_json("results_fullres.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

TEST_CASE("appx_add_experiments_randoml_res", "[exper_apprx_wi][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  nlohmann::json result_data;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "apprx_add_experiments_WI" / "fft16_apx.json";
  //  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "appx_add_experiments" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  //  auto tag_map = generate_tags_full_resolution(tb_module, vcd_data->get_timestamps().back(), stepsize);

  design.write_verilog(exported_verilog, params);
  for (uint32_t i = 3; i < 4; i++) {
    std::string vcd_file = "tb_fft16_ofdm_apx_";
    std::string heat_map_file = vcd_file;
    vcd_file += std::to_string(i) + ".vcd";
    auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "apprx_add_experiments" / vcd_file;
    VCDFileParser vcd_obj;
    auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());
    //    auto tag_map = generate_tags_random(tb_module, vcd_data->get_timestamps().back(), stepsize, 1000);
    const int tag_number = 500;
    auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tag_number, tb_module->get_ports());

    auto simulation_runtime_start = std::chrono::high_resolution_clock ::now();

    ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

    auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
    result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(simulation_runtime_end - simulation_runtime_start).count();

    uint32_t tag_cnt_outputs_total = 0;
    uint32_t tag_cnt_outputs = 0;
    uint32_t tag_cnt_ci_inputs = 0;
    std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
    std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
    std::vector<std::string> inputs_names = {"ci_add1_1", "ci_add1_2", "ci_sub1_1", "ci_sub1_2", "ci_add2_1", "ci_add2_2", "ci_sub2_1", "ci_sub2_2", "clk", "rstn", "ena", "xr", "xi"};
    std::vector<std::string> outputs_names = {"co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2", "qr", "qi"};
    std::vector<std::string> ci_outputs_names = {"co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2"};

    tag_cnt_outputs_total = tag_cnt_multiple_edges(instance, outputs_names);
    tag_cnt_outputs = tag_cnt_multiple_edges(instance, ci_outputs_names);
    const int index0 = 0;
    const int index1 = 1;
    const int index2 = 2;
    const int index3 = 3;
    const int index4 = 4;
    const int index5 = 5;
    const int index6 = 6;
    const int index7 = 7;
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index0]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index1]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index2]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index3]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index4]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index5]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index6]);
    tag_cnt_ci_inputs += tag_cnt_single_edge(instance, inputs_names[index7]);
    for (auto& name: inputs_names) {
      tag_cnt_inputs[name] = tag_cnt_single_edge(instance, name);
    }
    for (auto& o_name: outputs_names) {
      tag_cnt_output_1_input.clear();
      for (auto& i_name: inputs_names) {
        tag_cnt_output_1_input[i_name] = unique_tag_cnt_from_input_to_multiple_edges(instance, i_name, o_name);
      }
      tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
    }

    write_heat_map((heat_map_file + std::to_string(i) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names, outputs_names);

    result_data["injected_tags_@_ci_outputs_" + vcd_file] = tag_cnt_outputs;
    result_data["injected_tags_@_ci_inputs_" + vcd_file] = tag_cnt_ci_inputs;
    result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  }
  std::fstream os_json("results_random.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

//// not updated
//TEST_CASE("appx_mult", "[exper_apprx][.]") {
//  nlohmann::json params;
//  params["ift"] = true;
//  params["apprx"] = false;
//  params["apprx_mult"] = true;
//
//  const int stepsize = 500;
//  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_appx_mult" / "fft16_apx.json";
//  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_appx_mult" / "tb_fft16_ofdm_apx.vcd";
//  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
//  auto created = boost::filesystem::create_directories(temp_dir);
//  assert(created);
//
//  std::cout << temp_dir << '\n';
//  ducode::Design design = ducode::Design::parse_json(json_file);
//  VCDFileParser vcd_obj;
//  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());
//
//  auto exported_verilog = temp_dir / ("ift_design.v");
//  auto exported_testbench = temp_dir / "ift_testbench.v";
//  auto exported_tags = temp_dir / "vcd_input.txt";
//
//  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
////  auto tag_map = generate_tags_random(tb_module, vcd_data->get_timestamps().back(), stepsize, 0);
//  const int tag_number = 0;
//  auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tag_number, tb_module->get_ports());
//
//  design.write_verilog(exported_verilog, params);
//
//  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);
//}
//
//TEST_CASE("exact_mult", "[exper_apprx][.]") {
//  nlohmann::json params;
//  params["ift"] = true;
//  params["apprx"] = false;
//  params["apprx_mult"] = true;
//
//  const int stepsize = 500;
//  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_exact" / "fft16.json";
//  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_exact" / "tb_fft16_v2.vcd";
//  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
//  auto created = boost::filesystem::create_directories(temp_dir);
//  assert(created);
//
//  std::cout << temp_dir << '\n';
//  ducode::Design design = ducode::Design::parse_json(json_file);
//  VCDFileParser vcd_obj;
//  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());
//
//  auto exported_verilog = temp_dir / ("ift_design.v");
//  auto exported_testbench = temp_dir / "ift_testbench.v";
//  auto exported_tags = temp_dir / "vcd_input.txt";
//
//  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
////  auto tag_map = generate_tags_random(tb_module, vcd_data->get_timestamps().back(), stepsize, 0);
//  const int tag_number = 0;
//  auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tag_number, tb_module->get_ports());
//
//
//  design.write_verilog(exported_verilog, params);
//
//  auto simulation_runtime_start = std::chrono::high_resolution_clock ::now();
//
//  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);
//
//  auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
//  spdlog::info(std::chrono::duration_cast<std::chrono::milliseconds>(simulation_runtime_end - simulation_runtime_start).count());
//}
//
//TEST_CASE("appx_add_mult", "[exper_apprx][.]") {
//  nlohmann::json params;
//  params["ift"] = true;
//
//  const int stepsize = 500;
//  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_appx_add_mult" / "fft16_apx.json";
//  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_appx_add_mult" / "tb_fft16_ofdm_apx.vcd";
//  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
//  auto created = boost::filesystem::create_directories(temp_dir);
//  assert(created);
//
//  std::cout << temp_dir << '\n';
//  ducode::Design design = ducode::Design::parse_json(json_file);
//  VCDFileParser vcd_obj;
//  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());
//
//  auto exported_verilog = temp_dir / ("ift_design.v");
//  auto exported_testbench = temp_dir / "ift_testbench.v";
//  auto exported_tags = temp_dir / "vcd_input.txt";
//
//  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
//  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, "clk")), tb_module->get_ports());
//
//  design.write_verilog(exported_verilog, params);
//
//  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);
//}
//// END not updated

int main(int argc, char* argv[]) {
  // global setup...

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

  int result = Catch::Session().run(argc, argv);

  spdlog::shutdown();

  return result;
}