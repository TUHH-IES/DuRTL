/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/utility/verilog_parser.hpp>

#include <kernel/log.h>
#include <kernel/register.h>
#include <kernel/yosys.h>
#include <kernel/yosys_common.h>
#include <spdlog/spdlog.h>
#include <verible/common/strings/mem-block.h>
#include <verible/common/text/concrete-syntax-leaf.h>
#include <verible/common/text/concrete-syntax-tree.h>
#include <verible/common/text/parser-verifier.h>
#include <verible/common/text/text-structure.h>
#include <verible/common/text/visitors.h>
#include <verible/common/util/file-util.h>
#include <verible/verilog/CST/verilog-nonterminals.h>
#include <verible/verilog/analysis/verilog-analyzer.h>
#include <verible/verilog/parser/verilog-token-enum.h>
#include <verible/verilog/parser/verilog-token.h>

#include <cstdio>
#include <functional>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

std::string ducode::parse_verilog(const std::string& verilog_file, bool debug = false) {
  if (debug) {
    Yosys::log_files.push_back(stdout);
  }
  Yosys::log_error_stderr = true;
  Yosys::yosys_setup();
  Yosys::Design design;

  Yosys::Frontend::frontend_call(&design, nullptr, verilog_file, "verilog");

  Yosys::Pass::call(&design, "hierarchy -auto-top");
  Yosys::Pass::call(&design, "proc");
  Yosys::Pass::call(&design, "memory_collect");
  std::stringstream json_representation;
  Yosys::Backend::backend_call(&design, &json_representation, "", "json");

  return json_representation.str();
}

std::string ducode::parse_verilog(const std::vector<std::string>& verilog_files, bool debug = false) {
  if (debug) {
    Yosys::log_files.push_back(stdout);
  }
  Yosys::log_error_stderr = true;
  Yosys::yosys_setup();
  Yosys::Design design;

  for (const auto& verilog_file: verilog_files) {
    Yosys::Frontend::frontend_call(&design, nullptr, verilog_file, "verilog");
  }

  Yosys::Pass::call(&design, "hierarchy -auto-top");
  Yosys::Pass::call(&design, "proc");
  Yosys::Pass::call(&design, "memory_collect");
  std::stringstream json_representation;
  Yosys::Backend::backend_call(&design, &json_representation, "", "json");

  return json_representation.str();
}

namespace {
void VerifyParseTree(const verible::TextStructureView& text_structure) {
  const verible::ConcreteSyntaxTree& root = text_structure.SyntaxTree();
  if (root == nullptr) {
    throw std::runtime_error("Parse failed");
  }

  verible::ParserVerifier verifier(*root, text_structure.GetTokenStreamView());
  auto unmatched = verifier.Verify();

  if (!unmatched.empty()) {
    throw std::runtime_error("Unmatched tokens");
  }
}
}// namespace

struct InstancesFinder : verible::TreeVisitorRecursive {

  void Visit(const verible::SyntaxTreeNode& node) override {
    if (node.Tag() == verible::NodeTag(verilog::NodeEnum::kInstantiationBase)) {
      instances.emplace_back(&node);
    }
  }
  void Visit(const verible::SyntaxTreeLeaf& /*leaf*/) override {
  }

  std::vector<const verible::SyntaxTreeNode*> instances;
};

struct GateInstanceFinder : verible::TreeVisitorRecursive {

  void Visit(const verible::SyntaxTreeNode& node) override {
    if (node.Tag() == verible::NodeTag(verilog::NodeEnum::kGateInstance)) {
      gate_instances.emplace_back(&node);
    }
  }
  void Visit(const verible::SyntaxTreeLeaf& /*leaf*/) override {
  }

  std::vector<const verible::SyntaxTreeNode*> gate_instances;
};

struct SymbolsFinderRecursive : verible::TreeVisitorRecursive {

  void Visit(const verible::SyntaxTreeNode& /*node*/) override {
  }

  void Visit(const verible::SyntaxTreeLeaf& leaf) override {
    if (leaf.Tag() == verible::LeafTag(SymbolIdentifier)) {
      identifiers.emplace(leaf.get().text().to_string_view());
    }
  }

  std::unordered_set<std::string_view> identifiers;
};

struct SymbolsFinder : verible::SymbolVisitor {

  void Visit(const verible::SyntaxTreeNode& /*node*/) override {
  }

  void Visit(const verible::SyntaxTreeLeaf& leaf) override {
    if (leaf.Tag() == verible::LeafTag(SymbolIdentifier)) {
      identifiers.emplace(leaf.get().text().to_string_view());
    }
  }

  std::unordered_set<std::string_view> identifiers;
};

std::vector<std::string> ducode::get_instances_names(std::string_view filename, std::string_view module_name) {
  std::vector<std::string> instances_names;

  auto content_status = verible::file::GetContentAsMemBlock(filename);
  if (!content_status.status().ok()) {
    throw std::runtime_error(content_status.status().message().data());
  }
  std::shared_ptr<verible::MemBlock> content = std::move(*content_status);

  const verilog::VerilogPreprocess::Config preprocess_config;

  std::unique_ptr<verilog::VerilogAnalyzer> analyzer = verilog::VerilogAnalyzer::AnalyzeAutomaticMode(content, filename, preprocess_config);

  if (analyzer == nullptr) {
    throw std::runtime_error("Verilog analyzer cannot be null");
  }

  const auto lex_status = analyzer->LexStatus();
  const auto parse_status = analyzer->ParseStatus();

  if (!lex_status.ok() || !parse_status.ok()) {
    throw std::runtime_error("Parsing errors detected");
  }

  std::function<void(std::ostream&, size_t)> token_translator;

  token_translator = [](std::ostream& stream, size_t e) {
    stream << verilog::TokenTypeToString(e);
  };

  const verible::TokenInfo::Context context(analyzer->Data().Contents(), token_translator);

  const auto& text_structure = analyzer->Data();
  VerifyParseTree(text_structure);
  const auto& syntax_tree = text_structure.SyntaxTree();

  if (syntax_tree != nullptr) {
    auto instances_finder = std::make_unique<InstancesFinder>();
    syntax_tree->Accept(instances_finder.get());

    for (const auto* instance: instances_finder->instances) {
      auto symbols_finder_recursive = std::make_unique<SymbolsFinderRecursive>();
      (*instance)[0]->Accept(symbols_finder_recursive.get());

      if (!symbols_finder_recursive->identifiers.contains(module_name)) {
        continue;
      }

      spdlog::debug("Instance found: {}", module_name);

      auto gate_instance_finder = std::make_unique<GateInstanceFinder>();
      instance->Accept(gate_instance_finder.get());

      for (const auto* gate_instance: gate_instance_finder->gate_instances) {
        auto symbols_finder = std::make_unique<SymbolsFinder>();
        (*gate_instance)[0]->Accept(symbols_finder.get());
        for (const auto& identifier: symbols_finder->identifiers) {
          instances_names.emplace_back(identifier);
        }
      }
    }
  }

  return instances_names;
}