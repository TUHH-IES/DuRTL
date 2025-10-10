#pragma once

#include <boost/filesystem/path.hpp>
#include <gsl/narrow>
#include <vcd-parser/VCDTypes.hpp>

#include <utility>
#include <vector>

struct ExportedTagsFile {
  std::vector<VCDTime> time_points;// vector of time points
  boost::filesystem::path exported_tags_file_path;
};

struct TagTriggerConditionSignal {
  std::string signal_name;
  explicit TagTriggerConditionSignal(std::string signal) : signal_name(std::move(signal)) {}
  // For std::unordered_map
  bool operator==(const TagTriggerConditionSignal& other) const {
    return signal_name == other.signal_name;
  }
};

struct TagInjectTargetSignal {
  std::string signal_name;
  explicit TagInjectTargetSignal(std::string signal) : signal_name(std::move(signal)) {}
  // For std::unordered_map
  bool operator==(const TagInjectTargetSignal& other) const {
    return signal_name == other.signal_name;
  }
};

// Hash specialization
template<>
struct std::hash<TagInjectTargetSignal> {
  uint64_t operator()(const TagInjectTargetSignal& signal) const {
    return gsl::narrow<uint64_t>(std::hash<std::string>{}(signal.signal_name));
  }
};