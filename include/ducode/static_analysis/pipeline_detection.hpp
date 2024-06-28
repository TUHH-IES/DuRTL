/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/design.hpp>
#include <ducode/ids.hpp>

#include <boost/algorithm/string/join.hpp>

#include <iostream>
#include <set>
#include <vector>

using Confidence = float;

/** \brief Predefined patterns for different pipeline stages */
inline const std::vector<std::string> stage0 = {"instruction", "read", "fetch", "IF", "IR", "FET"};
inline const std::vector<std::string> stage1 = {"instruction", "dec", "decode", "ID"};
inline const std::vector<std::string> stage2 = {"operand", "read", "fetch", "OF", "OR", "memory", "MEM"};
inline const std::vector<std::string> stage3 = {"execute", "exe", "OE"};
inline const std::vector<std::string> stage4 = {"store", "write", "OS", "writeback", "WB"};
/*
// An alternative pattern set that finds arbitrary stuff mostly automatically generated internal nets
std::vector<std::string> stage0= { "0" };
std::vector<std::string> stage1= { "1" };
std::vector<std::string> stage2= { "2" };
std::vector<std::string> stage3= { "3" };
std::vector<std::string> stage4= { "4" };
std::vector<std::string> stage5= { "5" };
std::vector<std::string> stage6= { "6" };
*/

/** \brief Map from pipeline stages to typical patterns */
inline const std::vector<std::vector<std::string>> globalStagePatterns = {stage0, stage1, stage2, stage3, stage4};

inline std::string to_lower_copy(const std::string& input, const std::locale& loc = std::locale()) {
  auto const& facet = std::use_facet<std::ctype<char>>(loc);

  std::string out;
  out.reserve(input.size());

  std::transform(input.begin(), input.end(), std::back_inserter(out),
                 [&facet](auto c) { return facet.tolower(c); });

  return out;
}

namespace ducode {

/** \brief Class to store pipeline candidates

    A \c PipelineCandidate has multiple stages numbered starting from 0 for the first stage.
    Each stage holds a set of nets together with confidences of belonging to the respective stage.
    The pipeline candidate is bound to a fixed \c Design object as it uses pointers to \c CellModule
    to reference the actual implementation.
*/
class PipelineCandidate {
public:
  using ModuleNet = std::pair<const ducode::Module*, std::size_t>;
  using Stage = std::set<std::pair<ModuleNet, Confidence>>;
  std::vector<Stage> stages;

  [[nodiscard]] std::string print() const {

    auto print_stage = [](const Stage& stage) {
      std::vector<std::string> elements;
      for (const auto& [module_net, confidence]: stage) {
        const ducode::Module& module = *module_net.first;
        elements.emplace_back(fmt::format("\"{}.{}\", c {}", module.get_name(), module.get_nets()[gsl::narrow<std::size_t>(module_net.second)].get_name(), confidence));
      }
      return fmt::format("[ {} ]", boost::join(elements, ", "));
    };

    std::vector<std::string> elements;
    for (size_t i = 0; i < stages.size(); i++) {
      elements.emplace_back(fmt::format(" [ Stage_{} , [ {} ] ]", i, print_stage(stages[i])));
    }
    return fmt::format("Candidate: [ {} ]", boost::join(elements, ", "));
  }
};


inline std::ostream& operator<<(std::ostream& os, const PipelineCandidate& pc) {
  os << pc.print();
  return os;
}

/** \brief Class to identify potential pipelines by matching names of nets; only callable through constructor.

    Assumption: Desingers name signals in a pipeline stage with a consistent prefix/suffix/infix. 
    We consider a set of predefined patterns representing these.
    The class then collects each hypothesized stage in a separate \c PipelineCandidate with exactly that stage filled all other stages are empty.

    As a heurstic the longest pattern with most occurences is used per stage.
*/
class PipelineDetectionNames {
  /** \brief Parameter to choose which share of patterns found most often end up as pipeline candidates

      //  Defining a thresholdRatio. Maximum number of occurences \c max is counted. A nets belonging to a pattern with
          more than \c max * \c thersholdRatio occurences enter into a pipeline candidate.
      */
  static constexpr float thresholdRatio = 0.8f;
  /** \brief Minimal number of occurences of a pattern to be considered for a pipeline candidate. */
  static constexpr uint32_t minimalOccurrence = 2;

  Design const* m_design;

public:
  PipelineDetectionNames() = delete;

  /** \brief Given a design & a top module instantiating the design, a number of \c PipelineCandidate s is returned.

      These pipeline candidates are only valid wrt. to the actual design passed as a reference, becuase \c candidates refers
      to internals of the design using pointers.

      Each pipeline canidate only fills a single stage. These stages are numbered and may be combined to a full pipeline.
      In future, similar patterns "EXE" "DEC" versus "execute" "decode" may be combined into a single candidate with multiple stages.
      */
  explicit PipelineDetectionNames(const ducode::Design& design) : m_design(&design) {}

  [[nodiscard]] std::vector<PipelineCandidate> run() const {
    std::vector<PipelineCandidate> candidates;
    std::vector<std::vector<std::string>> stagePatterns;
    // Transform all given patterns to lower case
    const std::size_t nStages = globalStagePatterns.size();
    for (auto s = 0UL; s < globalStagePatterns.size(); s++) {
      stagePatterns.emplace_back();
      for (auto p = 0UL; p < globalStagePatterns[s].size(); p++) {
        stagePatterns[s].emplace_back(to_lower_copy(globalStagePatterns[s][p]));
      }
    }

    typedef int Support;
    // We create a 3D matrix:
    // Per net, per stage, per pattern we enter the support into the matrix
    std::vector<std::vector<std::vector<Support>>> net_stage_pat;
    // The nets are mapped to an index, so only nets with some support>0 get an entry. This index points into net_stage_pat.
    std::map<std::pair<const ducode::Module*, std::size_t>, std::size_t> net2index;
    // Per stage count number of occurences of each pattern
    std::vector<std::vector<unsigned int>> pCount(nStages);

    // for each module in design D
    for (const auto& module: m_design->get_modules()) {
      // for each net in module
      for (const auto& [index, net]: ranges::views::enumerate(module.get_nets())) {
        std::string nName = net.get_name();
        if (nName.empty()) {
          continue;
        }
        boost::algorithm::to_lower(nName);
        std::vector<std::vector<Support>> stage_pat(nStages);
        bool foundOneStage = false;
        // for each stage
        for (std::size_t s = 0; s < nStages; s++) {
          pCount[s].resize(stagePatterns[s].size(), 0);
          std::vector<Support> patSupport(stagePatterns[s].size());
          bool foundOnePattern = false;
          //for each possible pattern
          for (std::size_t p = 0; p < stagePatterns[s].size(); p++) {
            // calculate support
            const std::string& pattern = stagePatterns[s][p];
            // if net matches pattern
            if (nName.find(pattern) != std::string::npos) {
              foundOnePattern = true;
              // support is measured by length of matched pattern
              patSupport[p] = gsl::narrow<int>(pattern.length());
              pCount[s][p]++;
              spdlog::debug("Module: {} Net: {} contains pattern \" {} \"", module.get_name(), nName, pattern);
            }
          }
          if (foundOnePattern) {
            foundOneStage = true;
            stage_pat[s] = patSupport;
          }
        }
        if (foundOneStage) {
          std::size_t sz = net_stage_pat.size();
          net_stage_pat.resize(sz + 1);
          net_stage_pat[sz] = stage_pat;
          net2index[std::pair<const ducode::Module*, std::size_t>(&module, index)] = sz;
        }
      }
    }

    // Per stage determine the patterns with maximal occurrence
    // Select the longest ones with max. occurrence
    for (std::size_t s = 0; s < nStages; s++) {
      auto& ps = pCount[s];
      // Find pattern in stage s with max number of occurences
      auto it = max_element(ps.begin(), ps.end());
      if (it == ps.end()) {
        continue;
      }
      auto occMax = *it;
      if (occMax < minimalOccurrence) {
        continue;
      }
      // Find longest pattern with max number of occurences
      std::size_t lenMax = 0;
      for (std::size_t p = 0; p < ps.size(); p++) {
        auto cnt = ps[p];
        if (cnt == occMax) {
          const std::string pattern = stagePatterns[s][p];
          if (pattern.length() > lenMax) {
            lenMax = pattern.length();
          }
        }
      }
      auto threshold = static_cast<unsigned int>(thresholdRatio * static_cast<float>(occMax));
      PipelineCandidate::Stage stage;
      for (std::size_t p = 0; p < ps.size(); p++) {
        if (ps[p] > threshold) {
          Confidence confidence = (static_cast<float>(ps[p])) / static_cast<float>(occMax);
          std::cout << "Stage " << s << ": Selected pattern " << p << " with " << ps[p] << " occurences" << '\n';
          std::string pattern = stagePatterns[s][p];
          if (pattern.length() < lenMax) {
            std::cout << "Stage " << s << ": DROPPED - shorter than " << lenMax << '\n';
            continue;
          }
          for (auto& n2i: net2index) {//[std::pair<const Module *, NetID>(mp, n)]= sz;
            if (!net_stage_pat[n2i.second][s].empty() && net_stage_pat[n2i.second][s][p] > 0) {
              PipelineCandidate::Stage::value_type v(n2i.first, confidence);
              stage.insert(v);
            }
          }
        }
      }
      if (!stage.empty()) {
        PipelineCandidate pc;
        pc.stages.resize(s + 1);
        pc.stages[s] = stage;
        candidates.push_back(pc);
      }
    }

    return candidates;
  }
};
}// namespace ducode