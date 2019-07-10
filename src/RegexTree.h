#pragma once

#include <memory>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

class RegexTree {
 public:
  explicit RegexTree(std::string_view regex)
      : root(BuildTree(regex)), alphabet(Alphabet(root.get())) {}
  const std::unordered_set<std::size_t>& FirstPosRoot() const {}
  const std::unordered_set<char>& Alphabet() const { return alphabet; }
  const std::unordered_set<char>& FollowPos(int pos) const {}
  char CharAtPos(int pos) const {}

 private:
  class Node;
  using NodePtrSet = std::unordered_set<Node*>;

  enum class NodeType { CONCAT, UNION, STAR, LEAF };

  class Node {
   public:
    virtual ~Node() = 0;
    virtual const NodePtrSet& FirstPos() const final { return firstpos; }
    virtual const NodePtrSet& LastPos() const final { return lastpos; }
    virtual const NodePtrSet& FollowPos() const final { return followpos; }
    virtual bool Nullable() const final { return nullable; }

   protected:
    NodePtrSet firstpos;
    NodePtrSet lastpos;
    NodePtrSet followpos;
    bool nullable;
  };

  class ConcatNode : public Node {
   public:
    explicit ConcatNode(std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : left(std::move(l)), right(std::move(r)) {
      // firstpos = ;
      // lastpos = ;
      // followpos = ;
      nullable = left->Nullable() && right->Nullable();
    }

    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };

  class UnionNode : public Node {
   public:
    explicit UnionNode(std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : left(std::move(l)), right(std::move(r)) {
      // firstpos = ;
      // lastpos = ;
      // followpos = ;
      nullable = left->Nullable() || right->Nullable();
    }

    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };

  class StarNode : public Node {
   public:
    explicit StarNode(std::unique_ptr<Node> c) : child(std::move(c)) {
      // firstpos = ;
      // lastpos = ;
      // followpos = ;
      nullable = true;
    }

    std::unique_ptr<Node> child;
  };

  class LeafNode : public Node {
   public:
    explicit LeafNode(char l) : label(l) {
      // firstpos = ;
      // lastpos = ;
      // followpos = ;
      nullable = false;
    }

    char label;
  };

  std::unique_ptr<Node> BuildTree(std::string_view regex);
  std::unordered_set<char> Alphabet(Node* node);

  std::unique_ptr<Node> root;
  std::unordered_set<char> alphabet;
};
