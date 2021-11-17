// Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "open_spiel/abseil-cpp/absl/flags/flag.h"
#include "open_spiel/abseil-cpp/absl/flags/parse.h"
#include "open_spiel/abseil-cpp/absl/random/discrete_distribution.h"
#include "open_spiel/algorithms/cfr.h"
#include "open_spiel/algorithms/external_sampling_mccfr.h"
#include "open_spiel/algorithms/outcome_sampling_mccfr.h"
#include "open_spiel/algorithms/tabular_best_response_mdp.h"
#include "open_spiel/algorithms/tabular_exploitability.h"
#include "open_spiel/spiel.h"
#include "open_spiel/spiel_utils.h"

ABSL_FLAG(std::string, game_name, "db", "Game to run CFR on.");
ABSL_FLAG(int, num_iters, 10005, "How many iters to run for.");
ABSL_FLAG(int, report_every, 50, "How often to report.");

void PrintLegalActions(const open_spiel::State& state,
                       open_spiel::Player player,
                       const std::vector<open_spiel::Action>& movelist) {
  std::cerr << "Legal moves for player " << player << ":" << std::endl;
  for (open_spiel::Action action : movelist) {
    std::cerr << "  " << state.ActionToString(player, action) << std::endl;
  }
}

// Example code for using CFR+ to solve Kuhn Poker.
int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  std::shared_ptr<const open_spiel::Game> game =
      open_spiel::LoadGame(absl::GetFlag(FLAGS_game_name));
  // open_spiel::algorithms::CFRSolver solver(*game);
  open_spiel::algorithms::OutcomeSamplingMCCFRSolver solver(*game);
  // open_spiel::algorithms::ExternalSamplingMCCFRSolver solver(*game);
  std::cerr << "Starting (some kinda) CFR on " << game->GetType().short_name << "..." << std::endl;

  for (int i = 0; i < absl::GetFlag(FLAGS_num_iters); ++i) {
    // solver.EvaluateAndUpdatePolicy();
    solver.RunIteration();
    if (i % absl::GetFlag(FLAGS_report_every) == 0 ||
        i == absl::GetFlag(FLAGS_num_iters) - 1) {
      std::shared_ptr <open_spiel::Policy> average_policy = solver.AveragePolicy();
      open_spiel::algorithms::TabularBestResponseMDP tbr(*game, *average_policy);
      // TODO(WAN): Computing the NashConv takes an incredibly long time.
      // open_spiel::algorithms::TabularBestResponseMDPInfo br_info = tbr.NashConv();
      std::cout << i << std::endl; // << " " << br_info.nash_conv << std::endl;

      if (i % 50 == 0) {
        std::mt19937 rng(time(0));
        std::cerr << "NEW GAME WITH CURRENT POLICY" << std::endl;
        std::unique_ptr <open_spiel::State> state = game->NewInitialState();

        while (!state->IsTerminal()) {
          // std::cerr << "player " << state->CurrentPlayer() << std::endl;

          // Decision node, sample one uniformly.
          auto player = state->CurrentPlayer();

          std::vector <open_spiel::Action> actions = state->LegalActions();
          // PrintLegalActions(*state, player, actions);
          const auto &ap = solver.AveragePolicy()->GetStatePolicy(*state);
          std::vector<double> distribution;
          for (const auto &ape: ap) {
            distribution.emplace_back(ape.second);
          }
          absl::discrete_distribution<> dis(distribution.begin(), distribution.end());

          auto action = actions[dis(rng)];

          std::ostringstream ostr;
          ostr << "distribution[";
          for (const auto d: distribution) {
            ostr << d << ',';
          }
          ostr << "]";
          std::cerr << "\tChose action: " << state->ActionToString(player, action) << " " << ostr.str() << std::endl;

          state->ApplyAction(action);
        }

        std::cerr << "\tState: " << state->ToString() << std::endl;

        const auto &returns = state->Returns();
        for (auto p = open_spiel::Player{0}; p < game->NumPlayers(); p++) {
          std::cerr << "\tFinal return to player " << p << " is " << returns[p] << std::endl;
        }
      }
    }
  }
}
