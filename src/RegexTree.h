#pragma once

#include <memory>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

class RegexTree {
 public:
  explicit RegexTree(std::string_view regex) : root(BuildTree(regex)) {}

 private:
  class Node;
  using NodePtrSet = std::unordered_set<Node*>;

  enum class NodeType { CONCAT, UNION, STAR, LEAF };

  struct Node {
    virtual ~Node() = 0;
    virtual NodePtrSet FirstPos() const final { return firstpos; }
    virtual NodePtrSet LastPos() const final { return lastpos; }
    virtual NodePtrSet FollowPos() const final { return followpos; }
    virtual bool Nullable() const final { return nullable; }

    NodePtrSet firstpos;
    NodePtrSet lastpos;
    NodePtrSet followpos;
    bool nullable;
  };

  struct ConcatNode : public Node {
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

  struct UnionNode : public Node {
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

  struct StarNode : public Node {
   public:
    explicit StarNode(std::unique_ptr<Node> c) : child(std::move(c)) {
      // firstpos = ;
      // lastpos = ;
      // followpos = ;
      nullable = true;
    }

    std::unique_ptr<Node> child;
  };

  struct LeafNode : public Node {
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
  std::vector<Node*> leaves;
};
