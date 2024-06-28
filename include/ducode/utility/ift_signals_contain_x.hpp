//
// Created by gianluca on 20/03/2024.
//

#pragma once

#include <ducode/design.hpp>

#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>

namespace ducode {

inline bool ift_signals_contain_x(const VCDFile& data, const ducode::Design& design) {
  bool contains_x = false;

  const auto& signals = data.get_signals();

  for (const auto& module: design.get_modules()) {
    for (const auto& port: module.get_ports()) {
      auto signal_it = std::ranges::find_if(signals, [&](const VCDSignal& signal) { return signal.reference == (port.m_name + ift_tag); });
      if (signal_it == signals.end()) {
        throw std::runtime_error(fmt::format("{}{} signal not found", port.m_name, ift_tag));
      }
      auto signal_values = data.get_signal_values(signal_it->hash);
      for (const auto& signal_value: signal_values) {
        for (const auto& tag_bit: signal_value.value.get_value_vector()) {
          if (tag_bit == VCDBit::VCD_X) {
            spdlog::info("{} is the signal containing X", signal_it->reference);
            contains_x = true;
            break;
          }
        }
      }
    }
  }

  return contains_x;
}

}// namespace ducode