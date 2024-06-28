/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/utility/parser_utility.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <vcd-parser/VCDFileParser.hpp>

TEST_CASE("get_hierarchical_name", "[parser_utility]") {

  auto filepath = boost::filesystem::path{TESTFILES_DIR} / "parser_utility" / "get_hierarchical_name.vcd";
  REQUIRE(boost::filesystem::exists(filepath));

  VCDFileParser parser;
  auto vcdData = parser.parse_file(filepath.string());

  std::vector<std::string> testHierarchie = {"hierarchy2_tb", "t1", "s2", "b"};

  for (const auto& signal: vcdData->get_signals()) {
    if (signal.hash == "$") {
      if (signal.reference == "b") {
        REQUIRE(ducode::get_hierarchical_name(signal) == testHierarchie);
      } else {
        REQUIRE_FALSE(ducode::get_hierarchical_name(signal) == testHierarchie);
      }
    }
  }
}

TEST_CASE("bit_to_vector", "[parser_utility]") {

  const VCDTime testTime = 30;

  //testing function for a VCDBit
  VCDBit testBit = VCDBit::VCD_1;
  VCDValue bitValue(testBit);
  CHECK(bitValue.get_type() == VCDValueType::SCALAR);
  REQUIRE(ducode::bit_to_vector(bitValue).size() == 1);
  REQUIRE(ducode::bit_to_vector(bitValue).at(0) == VCDBit::VCD_1);

  VCDTimedValue timedBitValue{testTime, bitValue};
  REQUIRE(ducode::bit_to_vector(timedBitValue).size() == 1);
  REQUIRE(ducode::bit_to_vector(timedBitValue).at(0) == VCDBit::VCD_1);

  //testing function for a VCDBitVector
  VCDBitVector testVector = {VCDBit::VCD_0, VCDBit::VCD_0, VCDBit::VCD_1, VCDBit::VCD_1};
  VCDValue vectorValue(testVector);
  CHECK(vectorValue.get_type() == VCDValueType::VECTOR);
  REQUIRE(ducode::bit_to_vector(vectorValue) == vectorValue.get_value_vector());

  VCDTimedValue timedVectorValue{testTime, vectorValue};
  REQUIRE(ducode::bit_to_vector(timedVectorValue) == vectorValue.get_value_vector());

  //testing function for an empty VCDValue
  VCDValue emptyValue;
  CHECK(emptyValue.get_type() == VCDValueType::EMPTY);
  REQUIRE_THROWS_AS(ducode::bit_to_vector(emptyValue), std::invalid_argument);

  VCDTimedValue timedEmptyValue{testTime, emptyValue};
  REQUIRE_THROWS_AS(ducode::bit_to_vector(timedEmptyValue), std::invalid_argument);

  //testing function for an REAL vcdValue
  const VCDReal testValue = 4.9123;
  VCDValue realValue(testValue);
  CHECK(realValue.get_type() == VCDValueType::REAL);
  REQUIRE_THROWS_AS(ducode::bit_to_vector(realValue), std::invalid_argument);


  VCDTimedValue timedRealValue{testTime, realValue};
  REQUIRE_THROWS_AS(ducode::bit_to_vector(timedRealValue), std::invalid_argument);
}