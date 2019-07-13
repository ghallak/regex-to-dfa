#include "RegexTree.h"

#include <cstddef>
#include <memory>
#include <string_view>
#include <unordered_set>

RegexTree::Node::~Node() {}

std::unique_ptr<RegexTree::Node> RegexTree::BuildTree(std::string_view regex) {
  if (regex.length() == 1 || (regex.front() == '\\' && regex.length() == 2)) {
    auto leaf = std::make_unique<LeafNode>(
        leaves.size(), regex.length() == 1 ? regex[0] : regex[1]);
    leaves.emplace_back(leaf.get());
    return leaf;
  }

  // ...|...
  int open_parens = 0;
  for (std::size_t i = 0; i < regex.length(); ++i) {
    // don't consider "\(" or "\)" as regex parens
    if (regex[i] == '\\') {
      ++i;
      continue;
    }

    open_parens += regex[i] == '(';
    open_parens -= regex[i] == ')';
    if (regex[i] == '|' && open_parens == 0) {
      return std::make_unique<UnionNode>(BuildTree(regex.substr(0, i)),
                                         BuildTree(regex.substr(i + 1)));
    }
  }

  if (regex.front() == '(') {
    const auto close_paren_idx = [&] {
      int open_parens = 1;
      for (std::size_t i = 1; i < regex.length(); ++i) {
        open_parens += regex[i] == '(';
        open_parens -= regex[i] == ')';
        if (open_parens == 0) return i;
      }
      // unbalanced parens
      throw std::exception();
    }();

    // (...)*...
    if (close_paren_idx + 1 < regex.length() &&
        regex[close_paren_idx + 1] == '*') {
      // (...)*...
      if (close_paren_idx + 2 < regex.length()) {
        return std::make_unique<ConcatNode>(
            BuildTree(regex.substr(0, close_paren_idx + 2)),
            BuildTree(regex.substr(close_paren_idx + 2)));
      }
      // (...)*
      else {
        return std::make_unique<StarNode>(
            BuildTree(regex.substr(1, close_paren_idx - 1)));
      }
    }
    // (...)...
    else {
      // (...)...
      if (close_paren_idx + 1 < regex.length()) {
        return std::make_unique<ConcatNode>(
            BuildTree(regex.substr(1, close_paren_idx)),
            BuildTree(regex.substr(close_paren_idx + 1)));
      }
      // (...)
      else {
        return BuildTree(regex.substr(1, close_paren_idx));
      }
    }
  }

  // .*...
  if (regex.length() > 1 && regex[0] != '\\' && regex[1] == '*') {
    // .*...
    if (regex.length() > 2) {
      return std::make_unique<ConcatNode>(BuildTree(regex.substr(0, 2)),
                                          BuildTree(regex.substr(2)));
    }
    // .*
    else {
      return std::make_unique<StarNode>(BuildTree(regex.substr(0, 1)));
    }
  }

  // ...
  return std::make_unique<ConcatNode>(BuildTree(regex.substr(0, 1)),
                                      BuildTree(regex.substr(1)));
}

std::unordered_set<char> RegexTree::Alphabet(RegexTree::Node* n) {
  if (auto node = dynamic_cast<RegexTree::ConcatNode*>(n)) {
    auto alphabet1 = Alphabet(node->left.get());
    auto alphabet2 = Alphabet(node->right.get());
    alphabet1.insert(alphabet2.begin(), alphabet2.end());
    return alphabet1;
  } else if (auto node = dynamic_cast<RegexTree::UnionNode*>(n)) {
    auto alphabet1 = Alphabet(node->left.get());
    auto alphabet2 = Alphabet(node->right.get());
    alphabet1.insert(alphabet2.begin(), alphabet2.end());
    return alphabet1;
  } else if (auto node = dynamic_cast<RegexTree::StarNode*>(n)) {
    return Alphabet(node->child.get());
  } else if (auto node = dynamic_cast<RegexTree::LeafNode*>(n)) {
    return std::unordered_set<char>({node->label});
  } else {
    // there is no 5th type of RegexTree nodes
    throw std::exception();
  }
}

void RegexTree::CalcFollowPos(RegexTree::Node* n) {
  if (auto node = dynamic_cast<RegexTree::UnionNode*>(n)) {
    CalcFollowPos(node->left.get());
    CalcFollowPos(node->right.get());
  } else if (auto node = dynamic_cast<RegexTree::ConcatNode*>(n)) {
    for (auto i : node->left->lastpos) {
      leaves[i]->followpos.insert(node->right->firstpos.cbegin(),
                                  node->right->firstpos.cend());
    }
    CalcFollowPos(node->left.get());
    CalcFollowPos(node->right.get());
  } else if (auto node = dynamic_cast<RegexTree::StarNode*>(n)) {
    for (auto i : node->lastpos) {
      leaves[i]->followpos.insert(node->firstpos.cbegin(),
                                  node->firstpos.cend());
    }
    CalcFollowPos(node->child.get());
  }
}
