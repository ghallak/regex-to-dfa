#pragma once

#include "RegexTree.h"

class DFA {
 public:
  explicit DFA(const RegexTree& tree);
  void minimize();

 private:
  void update_dfa();
};
