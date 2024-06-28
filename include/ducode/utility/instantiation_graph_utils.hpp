//
// Created by gianluca on 25.10.23.
//

#pragma once

#include <ducode/instantiation_graph.hpp>

#include <string>

inline void write_filtered_graph(const std::string& filename, const DesignInstance& instance, const std::deque<DesignInstance::instantiation_graph::vertex_descriptor>& vertices_list) {
  // Predicate rules : keep any vertex which is part of the contained list; keep any edge whose source and target are part of the contained list

  std::function edge_predicate = [&](const DesignInstance::instantiation_graph::edge_descriptor& edge) {
    return (std::ranges::find(vertices_list, edge.m_target) != vertices_list.end() &&
            std::ranges::find(vertices_list, edge.m_source) != vertices_list.end());
  };

  std::function vertex_predicate = [&](const DesignInstance::instantiation_graph::vertex_descriptor& vertex) {
    return std::ranges::find(vertices_list, vertex) != vertices_list.end();
  };

  boost::filtered_graph<DesignInstance::instantiation_graph, decltype(edge_predicate), decltype(vertex_predicate)> filtered_graph(instance.m_graph, edge_predicate, vertex_predicate);
  auto file = std::ofstream(filename, std::ofstream::out);
  boost::write_graphviz(file, filtered_graph,
                        boost::make_label_writer(boost::get(&DesignInstance::VertexInfo::name, filtered_graph)),
                        boost::make_label_writer(boost::get(&DesignInstance::EdgeInfo::name, filtered_graph)));
  for (auto [it, it_end] = boost::edges(filtered_graph); it != it_end; it++) {
    spdlog::info("{}", filtered_graph[*it].name);
    for (const auto& tag_value: instance.m_vcd_data->get_signal_values(filtered_graph[*it].tags_signal->hash)) {
      std::stringstream line;
      for (const auto& tgs: tag_value.value.get_value_vector()) {
        line << tgs;
      }
      spdlog::info("{}", line.str());
    }
  }
}
