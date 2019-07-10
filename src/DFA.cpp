#include "DFA.h"
#include "RegexTree.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

DFA::DFA(const RegexTree& tree) {
  std::vector<
      std::pair<std::unique_ptr<State>, std::unordered_set<std::size_t>>>
      dstates;

  // add the initial state
  dstates.emplace_back(std::make_unique<State>(), tree.FirstPosRoot());

  auto& alphabet = tree.Alphabet();

  for (std::size_t i = 0; i < dstates.size(); ++i) {
    if (std::any_of(
            dstates[i].second.cbegin(), dstates[i].second.cend(),
            [sz = alphabet.size()](auto next_pos) { return next_pos == sz; })) {
      dstates[i].first->MakeAcceptState();
    }

    for (auto character : alphabet) {
      std::unordered_set<std::size_t> new_next_positions;
      for (auto next_pos : dstates[i].second) {
        if (tree.CharAtPos(next_pos) == character) {
          auto s = tree.FollowPos(next_pos);
          new_next_positions.insert(s.cbegin(), s.cend());
        }
      }

      //  // TODO: how can this happen?
      //  if (new_next_positions.empty()) continue;

      auto old_dstate = std::find_if(
          dstates.cbegin(), dstates.cend(),
          [& nnp = std::as_const(new_next_positions)](const auto& dstate) {
            return nnp == dstate.second;
          });

      if (old_dstate == dstates.end()) {
        dstates.emplace_back(std::make_unique<State>(), new_next_positions);
        dstates[i].first->AddTransition(dstates.back().first.get(), character);
      } else {
        dstates[i].first->AddTransition(old_dstate->first.get(), character);
      }
    }
  }

  // move the states ownership to the DFA
  for (auto& state : dstates) states.emplace_back(std::move(state.first));
}
