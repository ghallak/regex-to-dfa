#include "DFA.h"
#include "RegexTree.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

DFA::DFA(const RegexTree& tree) {
  auto create_state = [this] {
    states.emplace_back(std::make_unique<State>());
    return states.back().get();
  };

  set_alphabet(tree.Alphabet());

  std::vector<std::pair<State*, std::unordered_set<std::size_t>>> dstates;
  std::queue<std::size_t> q;

  dstates.emplace_back(create_state(), tree.firstpos_root());
  q.emplace(0);

  while (!q.empty()) {
    auto state = dstates[q.front()].first;
    auto leaves = dstates[q.front()].second;
    q.pop();

    for (auto leaf_pos : leaves) {
      // TODO: Create special symbol for end of augmented regex (#)
      if (tree.label(leaf_pos).to_string() == "#") {
        accept_states.emplace_back(state, tree.leaf_regex_id(leaf_pos));
        break;
      }
    }

    for (const auto& symbol : alphabet()) {
      std::unordered_set<std::size_t> new_leaves;
      for (auto leaf_pos : leaves) {
        if (tree.label(leaf_pos) == symbol) {
          new_leaves =
              utility::union_sets(new_leaves, tree.followpos(leaf_pos));
        }
      }

      if (new_leaves.empty()) continue;

      auto it = std::find_if(
          dstates.cbegin(), dstates.cend(),
          [new_leaves](auto elem) { return new_leaves == elem.second; });

      if (it == dstates.end()) {
        auto new_state = create_state();
        q.emplace(dstates.size());
        dstates.emplace_back(new_state, new_leaves);

        state->add_transition(new_state, symbol);
      } else {
        state->add_transition(it->first, symbol);
      }
    }
  }

  set_states_ids();
}
