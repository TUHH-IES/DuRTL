//
// Created by gianluca on 15.02.24.
//

#pragma once

#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>
#include <vcd-parser/VCDComparisons.hpp>
#include <vcd-parser/VCDFile.hpp>

namespace ducode {

inline bool no_repetitions_compare(const VCDSignalValues& a, const VCDSignalValues& b) {

  auto a_values = a |
                  ranges::views::transform([](const VCDTimedValue& value) { return value.value; }) |
                  ranges::views::unique;
  auto b_values = b |
                  ranges::views::transform([](const VCDTimedValue& value) { return value.value; }) |
                  ranges::views::unique;

  auto a_size = ranges::distance(a_values.begin(), a_values.end());
  auto b_size = ranges::distance(b_values.begin(), b_values.end());
  if (a_size != b_size) {
    return false;
  }

  auto a_it = a_values.begin();
  auto b_it = b_values.begin();
  for (; a_it != a_values.end() || b_it != b_values.end(); ++a_it, ++b_it) {
    if (*a_it != *b_it) {
      return false;
    }
  }


  return true;
}

inline bool no_repetitions_compare(const VCDFile& a, const VCDFile& b) {
  if (a.time_units != b.time_units) {
    return false;
  }

  if (a.time_resolution != b.time_resolution) {
    return false;
  }

  if (a.get_signals().size() != b.get_signals().size()) {
    return false;
  }

  if (a.get_timestamps().size() != b.get_timestamps().size()) {
    return false;
  }

  std::vector<std::reference_wrapper<const VCDSignal>> signals1(a.get_signals().begin(), a.get_signals().end());
  std::sort(signals1.begin(), signals1.end());

  std::vector<std::reference_wrapper<const VCDSignal>> signals2(b.get_signals().begin(), b.get_signals().end());
  std::sort(signals2.begin(), signals2.end());

  if (signals1 != signals2) {
    return false;
  }

  std::vector<std::reference_wrapper<const VCDScope>> scopes1(a.get_scopes().begin(), a.get_scopes().end());
  std::sort(scopes1.begin(), scopes1.end());

  std::vector<std::reference_wrapper<const VCDScope>> scopes2(b.get_scopes().begin(), b.get_scopes().end());
  std::sort(scopes2.begin(), scopes2.end());

  if (scopes1 != scopes2) {
    return false;
  }

  return std::ranges::all_of(signals1, [&](const auto& signal) { return no_repetitions_compare(a.get_signal_values(signal.get().hash), b.get_signal_values(signal.get().hash)); });
}

}// namespace ducode