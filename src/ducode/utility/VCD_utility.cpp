/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/design.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/legalize_identifier.hpp>
#include <ducode/utility/subrange.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
#include <boost/pending/property.hpp>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

const VCDScope& ducode::find_root_scope(const std::shared_ptr<VCDFile>& vcd_file, const Design& design) {
  const auto& module = design.get_module(design.get_top_module_name());

  std::set<std::string> signal_names;
  std::ranges::transform(module.get_ports(), std::inserter(signal_names, signal_names.begin()), [](const Port& port) { return legalize_identifier(port.get_name()); });

  auto all_signals_in_scope = [&](const VCDScope& scope) {
    return std::ranges::all_of(signal_names, [&scope](const std::string& name) {
      auto signal_count = std::ranges::count_if(scope.signals, [&](const VCDSignal* signal) {
        return signal->reference == name;
      });
      if (signal_count > 1) {
        throw std::runtime_error("Signal count exceeded 1");
      }
      return signal_count > 0;
    });
  };

  std::vector<std::reference_wrapper<const VCDScope>> scopes;

  for (const auto& scope: vcd_file->get_scopes()) {
    if (scope.name == "$root") {
      continue;
    }
    if (all_signals_in_scope(scope)) {
      scopes.emplace_back(scope);
    }
  }

  if (scopes.empty()) {
    throw std::runtime_error("No scope found");
  }
  if (scopes.size() == 1) {
    return scopes.front();
  }
  if (scopes.size() > 1) {
    // retry with net names
    std::vector<std::reference_wrapper<const VCDScope>> new_scopes;
    std::ranges::transform(module.get_nets(), std::inserter(signal_names, signal_names.begin()), [](const Net& net) { return legalize_identifier(net.get_name()); });
    for (const auto& scope: scopes) {
      if (all_signals_in_scope(scope)) {
        new_scopes.emplace_back(scope);
      }
    }
    if (new_scopes.size() == 1) {
      return new_scopes.front();
    }
  }
  if (boost::num_vertices(design.get_hierarchy_graph()) == 1) {
    const auto* scope = &vcd_file->get_scope("$root");
    const auto* tmp_scope = scope;
    while (tmp_scope != nullptr) {
      scope = tmp_scope;
      if (scope->children.empty()) {
        tmp_scope = nullptr;
      } else {
        tmp_scope = (scope->children[0]->type == VCDScopeType::VCD_SCOPE_MODULE) ? scope->children[0] : nullptr;
      }
    }
    return *scope;
  }

  using VertexProperty = boost::property<boost::vertex_name_t, std::string>;
  using Graph = boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, VertexProperty>;
  Graph vcd_graph{};
  std::unordered_map<Graph::vertex_descriptor, const VCDScope*> vertex_to_scope;

  std::function<void(const VCDScope&, Graph::vertex_descriptor)> populate_graph;
  populate_graph = [&vcd_graph, &vertex_to_scope, &populate_graph](const VCDScope& current_scope, Graph::vertex_descriptor current_vertex) {
    for (const auto* child: current_scope.children) {
      auto child_vertex = boost::add_vertex(child->name, vcd_graph);
      vertex_to_scope[child_vertex] = child;
      boost::add_edge(current_vertex, child_vertex, vcd_graph);
      populate_graph(*child, child_vertex);
    }
  };

  auto root_vertex = boost::add_vertex(vcd_file->root_scope->name, vcd_graph);
  vertex_to_scope[root_vertex] = vcd_file->root_scope;
  populate_graph(*vcd_file->root_scope, root_vertex);

  Graph::vertex_descriptor module_root_scope = 0;

  auto callback = [&](auto f, [[maybe_unused]] auto _) {
    Graph::vertex_descriptor module_root = 0;
    for (auto v: ducode::subrange(vertices(design.get_hierarchy_graph()))) {
      if (boost::get(boost::vertex_name_t(), design.get_hierarchy_graph(), v) == "root") {
        module_root = v;
        spdlog::debug("({}, {}) ", get(boost::vertex_index_t(), design.get_hierarchy_graph(), v), get(boost::vertex_index_t(), vcd_graph, get(f, v)));
      } else {
        if (get(boost::vertex_name_t(), design.get_hierarchy_graph(), v) != get(boost::vertex_name_t(), vcd_graph, get(f, v))) {
          return true;
        }
      }
    }
    module_root_scope = get(boost::vertex_index_t(), vcd_graph, get(f, module_root));
    return false;
  };

  auto file = std::ofstream("vcd_graph.dot", std::ofstream::out);
  boost::write_graphviz(file, vcd_graph,
                        boost::make_label_writer(boost::get(boost::vertex_name_t(), vcd_graph)));

  auto val = boost::vf2_subgraph_iso(design.get_hierarchy_graph(), vcd_graph, callback);

  if (!val) {
    throw std::runtime_error("Subgraph not found");
  }

  return *vertex_to_scope.at(module_root_scope);
}