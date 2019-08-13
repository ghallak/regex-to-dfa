#pragma once

#include <memory>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {
// this is used because the function RegexTree::FollowPos needs to return
// a const reference to a followpos set, so it can't return an empty set
// created locally in that function, since it will go out of scope once the
// function returns.
static const std::unordered_set<std::size_t> empty_set;
}  // namespace

class RegexTree {
 public:
  explicit RegexTree(std::string_view regex, std::string_view type)
      : root(ConcatEndNode(BuildTree(regex), type)),
        alphabet(Alphabet(root.get())) {
    CalcFollowPos(root.get());
  }

  /// Return an unordered_set of unique characters that exist in the regex.
  const std::unordered_set<char>& Alphabet() const { return alphabet; }

  /// Return FirstPos set for the root of the regex tree.
  const std::unordered_set<std::size_t>& FirstPosRoot() const {
    return root->firstpos;
  }

  /// Return the FollowPos set for a leaf in the regex tree given its position.
  const std::unordered_set<std::size_t>& FollowPos(std::size_t pos) const {
    return pos < leaves.size() ? leaves[pos]->followpos : empty_set;
  }

  /// Return true if the label of the leaf at the given position equals the
  /// given character, and return false otherwise.
  bool CharAtPos(char character, std::size_t pos) const {
    return pos < leaves.size() ? leaves[pos]->label == character : false;
  }

  /// Return the position of the end of the regex, which equals the number of
  /// leaves in the regex tree.
  std::size_t EndPos() const { return leaves.size(); }

 private:
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
      firstpos.insert(left->firstpos.cbegin(), left->firstpos.cend());
      if (left->nullable) {
        firstpos.insert(right->firstpos.cbegin(), right->firstpos.cend());
      }

      lastpos.insert(right->lastpos.cbegin(), right->lastpos.cend());
      if (right->nullable) {
        lastpos.insert(left->lastpos.cbegin(), left->lastpos.cend());
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
      firstpos.insert(left->firstpos.cbegin(), left->firstpos.cend());
      firstpos.insert(right->firstpos.cbegin(), right->firstpos.cend());

      lastpos.insert(left->lastpos.cbegin(), left->lastpos.cend());
      lastpos.insert(right->lastpos.cbegin(), right->lastpos.cend());

      nullable = left->nullable || right->nullable;
    }

    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };

  class StarNode : public Node {
   public:
    explicit StarNode(std::unique_ptr<Node> c) : child(std::move(c)) {
      firstpos = child->firstpos;
      lastpos = child->lastpos;
      nullable = true;
    }

    std::unique_ptr<Node> child;
  };

  class LeafNode : public Node {
   public:
    explicit LeafNode(std::size_t pos, char l) : label(l) {
      firstpos.emplace(pos);
      lastpos.emplace(pos);
      nullable = false;
    }

    std::unordered_set<std::size_t> followpos;
    char label;
  };

  class EndNode : public Node {
   public:
    explicit EndNode(std::size_t end_pos, std::string_view t) : type(t) {
      firstpos.insert(end_pos);
    }

   private:
    std::string_view type;
  };

  std::unique_ptr<Node> BuildTree(std::string_view regex, bool star = false);

  /// Create an EndNode and concatenate it with the root of the regex tree.
  std::unique_ptr<Node> ConcatEndNode(std::unique_ptr<RegexTree::Node> root,
                                      std::string_view type);

  std::unordered_set<char> Alphabet(Node* node);
  void CalcFollowPos(Node* node);

  std::vector<LeafNode*> leaves;
  std::unique_ptr<Node> root;
  std::unordered_set<char> alphabet;
};
