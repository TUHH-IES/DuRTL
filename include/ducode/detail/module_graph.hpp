/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <cstdint>
#include <string>

namespace ducode {

enum class VertexType : std::uint8_t {
  kCell,
  kModulePort,
  kCellPort,
  kNet,
  kUndefined
};

struct VertexInfo {
  std::string label;
  VertexType type = VertexType::kUndefined;
  VertexType port_type = VertexType::kUndefined;
  const Cell* cell_ptr = nullptr;// This is a non-owning pointer to a Cell.
  const Port* port_ptr = nullptr;// This is a non-owning pointer to a port.
  const Net* net_ptr = nullptr;  // This is a non-owning pointer to a Net.
};

struct EdgeInfo {
  std::string label;
};

using ModuleGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexInfo, EdgeInfo>;

}// namespace ducode
