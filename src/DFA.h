#pragma once

#include "RegexTree.h"

#include <vector>

class DFA {
 public:
  class State {
   public:
    void AddTransition(State* state, char c) {
      transitions.emplace_back(state, c);
    }
    bool IsAcceptState() const { return is_accept_state; }
    void MakeAcceptState() { is_accept_state = true; }

   private:
    class Transition {
     public:
      Transition(const State* s, char c) : state(s), character(c) {}

     private:
      const State* state;
      char character;
    };

    std::vector<Transition> transitions;
    bool is_accept_state;
  };

  explicit DFA(const RegexTree& tree);

 private:
  std::vector<std::unique_ptr<State>> states;
};
