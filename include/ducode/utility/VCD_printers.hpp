//
// Created by gianluca on 15.02.24.
//

#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <vcd-parser/VCDTimedValue.hpp>
#include <vcd-parser/VCDTypes.hpp>
#include <vcd-parser/VCDValue.hpp>

#include <algorithm>

template<>
struct fmt::formatter<VCDValue> : ostream_formatter {};

template<>
struct fmt::formatter<VCDTimedValue> : ostream_formatter {};

template<>
struct fmt::formatter<VCDSignalValues> : ostream_formatter {};