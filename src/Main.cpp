#include "DFA.h"
#include "RegexTree.h"

int main() {
  auto tree = RegexTree("(a|b)*abb");
  auto dfa = DFA(tree);
  dfa.CreateDotFile("out.gv");
}
