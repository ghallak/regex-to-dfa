#pragma once

#include <memory>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {
static const std::unordered_set<std::size_t> empty_set;
}

class RegexTree {
 public:
  explicit RegexTree(std::string_view regex)
      : root(BuildTree(regex)), alphabet(Alphabet(root.get())) {
    CalcFollowPos(root.get());

    // the loop below is added to avoid adding the nodes 'end' and 'concat_node'
    //
    //     concat_node
    //         /\
    //        /  \
    //       /    \
    //    root    end
    //
    // if the tree root is concatenated with the regex end node, and we run
    // CalcFollowPos(concat_node), the result will be equal to running the
    // following loop:
    for (auto i : root->lastpos) leaves[i]->followpos.emplace(EndPos());
  }
  const std::unordered_set<char>& Alphabet() const { return alphabet; }
  const std::unordered_set<std::size_t>& FirstPosRoot() const {
    return root->firstpos;
  }
  const std::unordered_set<std::size_t>& FollowPos(std::size_t pos) const {
    return pos < leaves.size() ? leaves[pos]->followpos : empty_set;
  }
  bool CharAtPos(char character, std::size_t pos) const {
    return pos < leaves.size() ? leaves[pos]->label == character : false;
  }
  std::size_t EndPos() const { return leaves.size(); }

 private:
  class Node;
  enum class NodeType { CONCAT, UNION, STAR, LEAF };

  class Node {
   public:
    virtual ~Node() = 0;

    std::unordered_set<std::size_t> firstpos;
    std::unordered_set<std::size_t> lastpos;
    bool nullable;
  };

  class ConcatNode : public Node {
   public:
    explicit ConcatNode(std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : left(std::move(l)), right(std::move(r)) {
      if (l->nullable) {
        firstpos.insert(l->firstpos.cbegin(), l->firstpos.cend());
        firstpos.insert(r->firstpos.cbegin(), r->firstpos.cend());
      } else {
        firstpos.insert(l->firstpos.cbegin(), l->firstpos.cend());
      }
      if (r->nullable) {
        lastpos.insert(l->lastpos.cbegin(), l->lastpos.cend());
        lastpos.insert(r->lastpos.cbegin(), r->lastpos.cend());
      } else {
        lastpos.insert(r->lastpos.cbegin(), r->lastpos.cend());
      }
      nullable = left->nullable && right->nullable;
    }

    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };

  class UnionNode : public Node {
   public:
    explicit UnionNode(std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : left(std::move(l)), right(std::move(r)) {
      firstpos.insert(l->firstpos.cbegin(), l->firstpos.cend());
      firstpos.insert(r->firstpos.cbegin(), r->firstpos.cend());
      lastpos.insert(l->lastpos.cbegin(), l->lastpos.cend());
      lastpos.insert(l->lastpos.cbegin(), l->lastpos.cend());
      nullable = left->nullable || right->nullable;
    }

    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };

  class StarNode : public Node {
   public:
    explicit StarNode(std::unique_ptr<Node> c) : child(std::move(c)) {
      firstpos = c->firstpos;
      lastpos = c->lastpos;
      nullable = true;
    }

    std::unique_ptr<Node> child;
  };

  class LeafNode : public Node {
   public:
    explicit LeafNode(std::size_t p, char l) : pos(p), label(l) {
      firstpos.emplace(pos);
      lastpos.emplace(pos);
      nullable = false;
    }

    std::unordered_set<std::size_t> followpos;
    std::size_t pos;
    char label;
  };

  std::unique_ptr<Node> BuildTree(std::string_view regex);
  std::unordered_set<char> Alphabet(Node* node);
  void CalcFollowPos(Node* node);

  std::unique_ptr<Node> root;
  std::unordered_set<char> alphabet;
  std::vector<LeafNode*> leaves;
};
