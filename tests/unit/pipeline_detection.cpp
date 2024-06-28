/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/static_analysis/pipeline_detection.hpp>

#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>

#include <vector>


/** \brief Test pipeline identification based on match string patterns for names of nets. Design does not contain any pipeline.
*/
TEST_CASE("pdnHierarchy", "[PipelineDetectionName]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "ift" / "hierarchy.json";
  auto d = ducode::Design::parse_json(json_file);

  const ducode::PipelineDetectionNames give_me_a_name(d);

  std::vector<ducode::PipelineCandidate> pc = give_me_a_name.run();

  //This design does not contain a pipeline canidate
  CHECK(pc.empty());
}

/** \brief Test pipeline identification based on match string patterns for names of nets. Design is a shift register resembing a pipeline. */
TEST_CASE("pdnSregPipeline", "[PipelineDetectionName]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "pipelines" / "sregPipeline_new.json";

  auto d = ducode::Design::parse_json(json_file);

  const ducode::PipelineDetectionNames pipelines(d);

  std::vector<ducode::PipelineCandidate> pc = pipelines.run();

  spdlog::debug("Pipeline candidates: \n");
  for (size_t pipeline = 0; pipeline < pc.size(); pipeline++) {
    const auto& candidate = pc[pipeline];
    //spdlog::debug("Candidate {}: {}\n",p,c);
    for (size_t stage = 0; stage < candidate.stages.size(); stage++) {
      if (stage != pipeline) {
        CHECK(candidate.stages[stage].empty());
      } else {
        CHECK(candidate.stages[stage].size() == 2);
      }
    }
  }
}

/** \brief Test pipeline identification based on match string patterns for names of nets. Design is a shift register resembing a pipeline. */
TEST_CASE("pdnPicorv", "[PipelineDetectionName]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "read_in" / "picorv32_2.json";

  //Temporalily deactivating test case to fix reading of picorv32.json

  auto d = ducode::Design::parse_json(json_file);

  const ducode::PipelineDetectionNames pipelines(d);
  std::vector<ducode::PipelineCandidate> pc = pipelines.run();

  spdlog::debug("Calculated pipeline candidates");

  spdlog::debug("Pipeline candidates: \n");
  for (const auto& c: pc) {
    CHECK(!c.stages.empty());
    //spdlog::debug("{}\n",c);
  }

  //This design does not contain a pipeline canidate
  CHECK(!pc.empty());
}
