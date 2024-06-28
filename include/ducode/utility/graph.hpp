//
// Created by gianluca on 10/03/2024.
//

#pragma once

#include <ducode/detail/module_graph.hpp>
#include <ducode/utility/contains.hpp>
#include <ducode/utility/subrange.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>

#include <vector>

namespace ducode {

template<typename GraphType>
inline bool follow_path(const GraphType& graph, typename GraphType::vertex_descriptor current_vertex, std::vector<typename GraphType::vertex_descriptor> path, bool print = false) {
  if (graph[current_vertex].type == VertexType::kNet && contains(path, current_vertex)) {
    if (print) {
      std::vector<std::string> string_path;
      string_path.reserve(path.size());
      for (const auto& vertex: path) {
        string_path.emplace_back(graph[vertex].label);
      }
      spdlog::info("Found loop: [{}]", boost::algorithm::join(string_path, ", "));
    }
    return true;
  }

  bool return_value = false;

  path.emplace_back(current_vertex);

  auto vertices = ducode::subrange(boost::adjacent_vertices(current_vertex, graph));
  for (const auto& vertex: vertices) {
    if (graph[vertex].type == VertexType::kNet) {
      if (follow_path(graph, vertex, path, print)) {
        return_value = true;
      }
    }
  }

  return return_value;
}

template<typename GraphType>
inline bool contains_loops(const GraphType& graph) {
  bool return_value = false;
  auto vertices = ducode::subrange(boost::vertices(graph));
  for (const auto& vertex: vertices) {
    if (graph[vertex].type == VertexType::kNet) {
      if (follow_path(graph, vertex, std::vector<typename GraphType::vertex_descriptor>{})) {
        return_value = true;
      }
    }
  }

  return return_value;
}

template<typename GraphType>
inline void print_loops(const GraphType& graph) {
  auto vertices = ducode::subrange(boost::vertices(graph));
  for (const auto& vertex: vertices) {
    if (graph[vertex].type == VertexType::kNet) {
      follow_path(graph, vertex, std::vector<typename GraphType::vertex_descriptor>{}, true);
    }
  }
}

template<typename GraphType>
inline bool follow_path(const GraphType& graph, typename GraphType::vertex_descriptor current_vertex, typename GraphType::vertex_descriptor source_vertex) {
  if (graph[current_vertex].type == VertexType::kNet && current_vertex == source_vertex) {
    return true;
  }

  bool return_value = false;

  auto vertices = ducode::subrange(boost::adjacent_vertices(current_vertex, graph));
  for (const auto& vertex: vertices) {
    if (graph[vertex].type == VertexType::kNet) {
      if (follow_path(graph, vertex, source_vertex)) {
        return_value = true;
      }
    }
  }

  return return_value;
}

template<typename GraphType>
inline bool source_reachable(const GraphType& graph, typename GraphType::vertex_descriptor source, typename GraphType::vertex_descriptor target) {
  return follow_path(graph, target, source);
}

}// namespace ducode