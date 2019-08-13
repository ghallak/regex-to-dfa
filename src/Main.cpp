#include "DFA.h"
#include "RegexTree.h"

int main() {
  auto tree = RegexTree("(a|b)*abb", "match");
  auto dfa = DFA(tree);
  dfa.CreateDotFile("out.gv");
}
