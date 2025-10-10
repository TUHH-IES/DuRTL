#pragma once

#include <ducode/cell.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <gsl/narrow>
#include <vcd-parser/VCDTimedValue.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace ducode {

enum class SignalValueType : std::uint8_t {
  SCALAR,//!< Single Bit
  VECTOR,//!< Vector of Bit
  REAL,  //!< IEEE Floating point (64bit).
  EMPTY
};

enum class SignalBit : std::uint8_t {
  BIT_0 = 0,//!< Logic zero
  BIT_1 = 1,//!< Logic one
  BIT_X = 2,//!< Unknown / Undefined
  BIT_Z = 3 //!< High Impedence.
};

//! A vector of VCDBit values.
//! Warning: This vector is in reverse order - the MSB has index 0 - this is in line with the storage in VCD strings.
using SignalBitVector = std::vector<SignalBit>;

//! Typedef to identify a real number as stored in a VCD.
using SignalReal = double;

using SignalName = std::string;
using SignalSize = uint64_t;
using SignalHash = std::string;

struct SignalInfo {
  SignalName reference;
  SignalSize size = 0;
  SignalHash hash;

  SignalInfo() = default;
  explicit SignalInfo(VCDSignal& signal) : reference(signal.reference), size(gsl::narrow<uint64_t>(signal.size)), hash(signal.hash){};
};

class SignalValue {

  //! The type of value this instance stores.
  SignalValueType type = SignalValueType::EMPTY;

  //! The actual value stored, as identified by type.
  std::variant<SignalBit, SignalBitVector, SignalReal> m_value;

public:
  //! Convert a SignalBit to a single char
  static char signal_bit_to_char(SignalBit bit) {
    switch (bit) {
      case (SignalBit::BIT_0):
        return '0';
      case (SignalBit::BIT_1):
        return '1';
      case (SignalBit::BIT_Z):
        return 'Z';
      case (SignalBit::BIT_X):
      default:
        return 'X';
    }
  }

  SignalValue() = default;

  /*!
    @brief Create a new VCDValue with the type VCD_SCALAR
    */
  explicit SignalValue(SignalBit value) : type(SignalValueType::SCALAR), m_value(value) {}

  /*!
    @brief Create a new VCDValue with the type VCD_VECTOR
    */
  explicit SignalValue(const SignalBitVector& value) : type(SignalValueType::VECTOR), m_value(value) {
  }

  /*!
    @brief Create a new VCDValue with the type VCD_VECTOR
    */
  explicit SignalValue(SignalReal value) : type(SignalValueType::REAL), m_value(value) {
  }

  /*!
    @brief A bitstring has the form "#b0100", so the MSB follows after the prefix "#b". This order is the same as in 
           VCD strings.
  */
  explicit SignalValue(const std::string& bitstring) {
    if (bitstring.size() < 3) {
      throw std::out_of_range("SignalValue - trying to initialize with empty bitstring!");
    }
    if (bitstring[0] != '#' || (bitstring[1] != 'b' && bitstring[1] != 'x')) {
      throw std::out_of_range(std::string("SignalValue - trying to initialize with non-bitvalued string: ") + bitstring);
    }
    if (bitstring[1] == 'b') {
      if (bitstring.size() == 3) {
        if (bitstring[2] != '1' && bitstring[2] != '0') {
          throw std::out_of_range(std::string("SignalValue - trying to initialize with non-bitvalued string: ") + bitstring);
        }
        m_value = SignalBit::BIT_0;
        if (bitstring[2] == '1') {
          m_value = SignalBit::BIT_1;
        }
        type = SignalValueType::SCALAR;
        return;
      }

      SignalBitVector bit_vector;
      bit_vector.reserve(bitstring.size());
      for (size_t index = 2; index < bitstring.size(); index++) {
        char bitchar = bitstring[index];
        if (bitchar == '0') {
          bit_vector.push_back(SignalBit::BIT_0);
        } else if (bitchar == '1') {
          bit_vector.push_back(SignalBit::BIT_1);
        } else {
          throw std::out_of_range(std::string("SignalValue - trying to initialize with non-bitvalued string!") + bitstring);
        }
      }
      m_value = bit_vector;
      type = SignalValueType::VECTOR;
      return;
    }
    throw std::out_of_range(std::string("SignalValue - conversion to SignalValue not implemented for this bitstring: ") + bitstring);
  }

  explicit SignalValue(const VCDValue& vcd_value) {
    switch (vcd_value.get_type()) {
      case VCDValueType::SCALAR: {
        // Convert VCD_SCALAR to SignalBit
        SignalBit bit = SignalBit::BIT_X;// Initialize with default value
        switch (vcd_value.get_value_bit()) {
          case VCDBit::VCD_0:
            bit = SignalBit::BIT_0;
            break;
          case VCDBit::VCD_1:
            bit = SignalBit::BIT_1;
            break;
          case VCDBit::VCD_Z:
            bit = SignalBit::BIT_Z;
            break;
          case VCDBit::VCD_X:
          default:
            bit = SignalBit::BIT_X;
            break;
        }
        m_value = bit;
        type = SignalValueType::SCALAR;
        break;
      }
      case VCDValueType::VECTOR: {
        // Convert VCD_VECTOR to SignalBitVector
        SignalBitVector bit_vector;
        bit_vector.reserve(vcd_value.get_value_vector().size());
        for (auto& vcdValue: vcd_value.get_value_vector()) {
          SignalBit bit = SignalBit::BIT_X;
          switch (vcdValue) {
            case VCDBit::VCD_0:
              bit = SignalBit::BIT_0;
              break;
            case VCDBit::VCD_1:
              bit = SignalBit::BIT_1;
              break;
            case VCDBit::VCD_Z:
              bit = SignalBit::BIT_Z;
              break;
            case VCDBit::VCD_X:
            default:
              bit = SignalBit::BIT_X;
              break;
          }
          bit_vector.push_back(bit);
        }
        m_value = bit_vector;
        type = SignalValueType::VECTOR;
        break;
      }
      case VCDValueType::REAL: {
        // Convert VCD_REAL to SignalReal
        m_value = vcd_value.get_value_real();
        type = SignalValueType::REAL;
        break;
      }
      case VCDValueType::EMPTY:
        break;
    }
  }

  //! Return the type of value stored by this class instance.
  [[nodiscard]] SignalValueType get_type() const {
    return type;
  }

  //! Get the bit value of the instance.
  [[nodiscard]] SignalBit get_value_bit() const {
    return std::get<SignalBit>(m_value);
  }

  //! Get the vector value of the instance.
  [[nodiscard]] const SignalBitVector& get_value_vector() const {
    return std::get<SignalBitVector>(m_value);
  }

  //! Get the real value of the instance.
  [[nodiscard]] SignalReal get_value_real() const {
    return std::get<SignalReal>(m_value);
  }

  // Equality operator
  bool operator==(const SignalValue& other) const {
    return type == other.type && m_value == other.m_value;
  }
  // Inequality operator
  bool operator!=(const SignalValue& other) const {
    return !(*this == other);
  }

  // Output
  [[nodiscard]] std::string to_string() const {
    std::string result;
    switch (type) {
      case SignalValueType::SCALAR:
        result += signal_bit_to_char(get_value_bit());
        break;
      case SignalValueType::VECTOR: {
        const SignalBitVector& sbv = get_value_vector();
        const auto len = sbv.size();
        for (auto pos = len - 1; pos < len; pos--) {
          result += signal_bit_to_char(sbv.at(pos));
        }
        /*
          for (const auto &bit : boost::adaptors::reverse(get_value_vector())) {
            result += signal_bit_to_char(bit);
          }*/
      } break;
      default:
        throw std::runtime_error("String conversion for value type not implemented, yet!");
    }
    return result;
  }
};

using SignalTime = int64_t;// We use a signed number to account for negative delta values or negative time steps in formal

struct TimedSignalValue {
  SignalTime time{};
  SignalValue value;
};

struct SignalValues {
  std::deque<TimedSignalValue> m_timed_signal_values;
  SignalValues() = default;
  explicit SignalValues(const VCDSignalValues& vcd_values) {
    for (const auto& vcd_timed_value: vcd_values) {
      SignalValue signal_value(vcd_timed_value.value);
      m_timed_signal_values.emplace_back(TimedSignalValue(vcd_timed_value.time, signal_value));
    }
  }
  [[nodiscard]] bool empty() const { return m_timed_signal_values.empty(); }
};

struct InstantiationGraphVertexInfo {
  std::string name;
  std::string type;
  std::string parent_module;
  std::vector<std::string> hierarchy;
  const Cell* cell_ptr = nullptr;// This is a non-owning pointer to a Cell.
  const Port* port_ptr = nullptr;// This is a non-owning pointer to a port.
};

struct InstantiationGraphEdgeInfo {
  std::string name;
  std::string parent_module;
  std::vector<std::string> hierarchy;
  const Port* source_port_ptr = nullptr;
  const Port* target_port_ptr = nullptr;
  const Net* net_ptr = nullptr;
};

using instantiation_graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, InstantiationGraphVertexInfo, InstantiationGraphEdgeInfo>;

inline const std::set<std::string>& get_excluded_signals() {
  static const std::set<std::string> excluded_signals = {"clk", "slaveSCLK", "sys_clk"};
  return excluded_signals;
}
inline const std::unordered_set<std::string>& get_dff_types() {
  static const std::unordered_set<std::string> dff_types{
      "$dff", "$adff", "$adffe", "$adlatch", "$aldff",
      "$aldffe", "$dffe", "$sdff", "$sdffe", "$dlatch",
      "$dlatchsr", "$dffsr", "$dffsre"};
  return dff_types;
}

struct BitVector : public std::vector<Bit> {
  using std::vector<Bit>::vector;// Inherit constructors
};

struct InputEdge {
  instantiation_graph::edge_descriptor edge;
};

struct TargetEdge {
  instantiation_graph::edge_descriptor edge;
};

struct SignalIdentifier {
  std::string name;
  std::vector<std::string> hierarchy;

  // Equality operator
  bool operator==(const SignalIdentifier& other) const {
    return name == other.name && hierarchy == other.hierarchy;
  }
  // Inequality operator
  bool operator!=(const SignalIdentifier& other) const {
    return !(*this == other);
  }
  // Output
  [[nodiscard]] std::string to_string() const {
    std::string fullname;
    for (const auto& modulename: hierarchy) {
      fullname += modulename + ".";
    }
    fullname += name;
    return fullname;
  }
};

struct IFInfo {
  SignalIdentifier input_id;
  SignalTime tag_injection_time;
  SignalTime tag_observation_time;
  IFInfo() : tag_injection_time(0),
             tag_observation_time(0){};
  IFInfo(SignalIdentifier& name, const SignalTime& injection_time, const SignalTime& observation_time) : input_id(std::move(name)),
                                                                                                         tag_injection_time(injection_time),
                                                                                                         tag_observation_time(observation_time){};

  bool operator==(const IFInfo& other) const = default;
};

inline std::size_t hash_value(const IFInfo& info) {
  return std::hash<std::string>{}(info.input_id.name) ^ std::hash<SignalTime>{}(info.tag_injection_time) ^ std::hash<SignalTime>{}(info.tag_observation_time);
}


class TagSourceList {
public:
  std::vector<IFInfo> m_tag_vector;

  TagSourceList() = default;
  explicit TagSourceList(std::vector<IFInfo> tag_vector) : m_tag_vector(std::move(tag_vector)){};
};

class TagList {
public:
  std::vector<std::pair<SignalTime, ducode::Port>> m_tag_vector;

  TagList() = default;
  explicit TagList(std::vector<std::pair<SignalTime, ducode::Port>> tag_vector) : m_tag_vector(std::move(tag_vector)){};
};

}// namespace ducode

template<>
struct std::hash<ducode::SignalIdentifier> {
  size_t operator()(const ducode::SignalIdentifier& id) const {
    size_t seed = boost::hash<std::string>{}(id.name);
    // Combine with hierarchy elements
    for (const auto& part: id.hierarchy) {
      boost::hash_combine(seed, part);
    }
    return seed;
  }
};

template<>
struct std::hash<ducode::IFInfo> {
  std::size_t operator()(const ducode::IFInfo& info) const {
    boost::hash<ducode::IFInfo> hasher;
    return hasher(info);
  }
};
