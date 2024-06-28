//
// Created by Ludwig on 06/03/2023.
// Copyright (c) 2023 TUHH. All rights reserved.
//

#pragma once

#include <vcd-parser/VCDFileParser.hpp>

namespace ducode {

//function gives a vector with {signal->lastParent->name, ...,sinal->parent->name, signal->name}
inline std::vector<std::string> get_hierarchical_name(const VCDSignal& signal) {

  std::vector<std::string> hierarchy;
  VCDScope* currScope = signal.scope;

  hierarchy.push_back(signal.reference);

  while (currScope->parent != nullptr) {
    hierarchy.push_back(currScope->name);
    currScope = currScope->parent;
  }

  std::reverse(std::begin(hierarchy), std::end(hierarchy));

  return hierarchy;
}

//returns a vector even is the VCDValue is a SCALAR
inline VCDBitVector bit_to_vector(const VCDTimedValue& timed_value) {
  VCDBitVector valVector;
  if (timed_value.value.get_type() == VCDValueType::SCALAR) {
    valVector.push_back(timed_value.value.get_value_bit());
  } else if (timed_value.value.get_type() == VCDValueType::VECTOR) {
    valVector = timed_value.value.get_value_vector();
  } else {
    throw std::invalid_argument("should not be called on EMPTY or REAL");
  }
  return valVector;
}

inline VCDBitVector bit_to_vector(const VCDValue& value) {
  VCDBitVector valVector;
  if (value.get_type() == VCDValueType::SCALAR) {
    valVector.push_back(value.get_value_bit());
  } else if (value.get_type() == VCDValueType::VECTOR) {
    valVector = value.get_value_vector();
  } else {
    throw std::invalid_argument("should not be called on EMPTY or REAL");
  }
  return valVector;
}

}// namespace ducode