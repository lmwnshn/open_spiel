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

#include "open_spiel/games/db.h"

#include <algorithm>
#include <chrono>
#include <memory>
#include <regex>
#include <utility>
#include <vector>
#include <stdexcept>

#include "open_spiel/spiel_utils.h"
#include "open_spiel/utils/tensor_view.h"

namespace open_spiel {
namespace db {
namespace {

bool IsClient(const Player player) {
  return player == 0;
}

bool IsServer(const Player player) {
  return player == 1;
}

struct EstCost {
  public:
    EstCost(pqxx::result *rset) {
      for (const auto &r : *rset) {
        for (const auto &f: r) {
          std::smatch matches;
          std::string current_str{pqxx::to_string(f)};

          const std::regex REGEXP{".*\\(cost=(\\d+\\.?\\d+)\\.\\.(\\d+\\.?\\d+) rows=(\\d+) width=(\\d+)\\)"};
          if (std::regex_match(explain_str, matches, REGEXP)) {
            startup_cost_ = std::stod(matches[1].str());
            total_cost_ = std::stod(matches[2].str());
            num_rows_ = std::stol(matches[3].str());
            width_ = std::stol(matches[4].str());
            return;
          }
        }
      }
      throw std::runtime_error("EC: Bad input resultset");
    }

    friend std::ostream& operator<<(std::ostream& os, const EstCost& cost) {
      os << "[EC(";
      os << cost.startup_cost_ << ',';
      os << cost.total_cost_ << ',';
      os << cost.num_rows_ << ',';
      os << cost.width_ << ")]";
      return os;
    }

  public:
    double startup_cost_ = -1;
    double total_cost_ = -1;
    long num_rows_ = -1;
    long width_ = -1;
};

struct TrueCost {
  public:
    TrueCost(pqxx::result *rset) {
      bool done[3] = {false, false, false};
      for (const auto &r : *rset) {
        for (const auto &f: r) {
          std::smatch matches;
          std::string current_str{pqxx::to_string(f)};

          if (!done[0]) {
            const std::regex REGEXP{
                    ".*\\(cost=(\\d+\\.?\\d+)\\.\\.(\\d+\\.?\\d+) rows=(\\d+) width=(\\d+)\\).*\\(actual time=(\\d+\\.?\\d+)\\.\\.(\\d+\\.?\\d+) rows=(\\d+) loops=(\\d+)\\)"};
            if (std::regex_match(current_str, matches, REGEXP)) {
              startup_cost_ = std::stod(matches[1].str());
              total_cost_ = std::stod(matches[2].str());
              num_rows_ = std::stol(matches[3].str());
              width_ = std::stol(matches[4].str());
              actual_startup_time_ = std::stod(matches[5].str());
              actual_total_time_ = std::stod(matches[6].str());
              actual_num_rows_ = std::stol(matches[7].str());
              actual_loops_ = std::stol(matches[8].str());
              done[0] = true;
              continue;
            }
          }

          if (!done[1]) {
            const std::regex REGEXP_PLAN{"Planning Time: (\\d+\\.?\\d+) ms"};
            if (std::regex_match(current_str, matches, REGEXP_PLAN)) {
              actual_planning_time_ms_ = std::stod(matches[1].str());
              done[1] = true;
              continue;
            }
          }

          if (!done[2]) {
            const std::regex REGEXP_EXEC{"Execution Time: (\\d+\\.?\\d+) ms"};
            if (std::regex_match(current_str, matches, REGEXP_EXEC)) {
              actual_execution_time_ms_ = std::stod(matches[1].str());
              done[2] = true;
              return;
            }
          }
        }
      }
      throw std::runtime_error("TC: Bad input resultset.");
    }

    friend std::ostream& operator<<(std::ostream& os, const TrueCost& cost) {
      os << "[TC(";
      os << cost.startup_cost_ << ',';
      os << cost.total_cost_ << ',';
      os << cost.num_rows_ << ',';
      os << cost.width_ << ',';
      os << cost.actual_startup_time_ << ',';
      os << cost.actual_total_time_ << ',';
      os << cost.actual_num_rows_ << ',';
      os << cost.actual_loops_ << ',';
      os << cost.actual_planning_time_ms_ << ',';
      os << cost.actual_execution_time_ms_ << ")]";
      return os;
    }

  public:
    double startup_cost_ = -1;
    double total_cost_ = -1;
    long num_rows_ = -1;
    long width_ = -1;
    double actual_startup_time_ = -1;
    double actual_total_time_ = -1;
    long actual_num_rows_ = -1;
    long actual_loops_ = -1;
    double actual_planning_time_ms_ = -1;
    double actual_execution_time_ms_ = -1;
};

// Facts about the game.
const GameType kGameType{
    /*short_name=*/"db",
    /*long_name=*/"DB",
    GameType::Dynamics::kSequential,
    GameType::ChanceMode::kDeterministic,
    GameType::Information::kPerfectInformation,
    GameType::Utility::kZeroSum,
    GameType::RewardModel::kTerminal,
    /*max_num_players=*/2,
    /*min_num_players=*/2,
    /*provides_information_state_string=*/true,
    /*provides_information_state_tensor=*/false,
    /*provides_observation_string=*/true,
    /*provides_observation_tensor=*/false,
    /*parameter_specification=*/{}  // no parameters
};

std::shared_ptr<const Game> Factory(const GameParameters& params) {
  return std::shared_ptr<const Game>(new DbGame(params));
}

REGISTER_SPIEL_GAME(kGameType, Factory);

}  // namespace

void DbState::DoApplyAction(Action move) {
  current_player_ = 1 - current_player_;
  num_moves_ += 1;
}

std::vector<Action> DbState::LegalActions() const {
  if (IsTerminal()) return {};
  std::vector<Action> moves;

  const auto &ca = game_->GetClientActions();
  const auto &sa = game_->GetServerActions();

  if (IsServer(current_player_)) {
    moves.reserve(sa.size());
    for (size_t i = 0 ; i < sa.size(); ++i) {
      moves.emplace_back(i);
    }
  } else {
    SPIEL_CHECK_TRUE(IsClient(current_player_));
    moves.reserve(ca.size());
    for (size_t i = 0 ; i < ca.size(); ++i) {
      moves.emplace_back(i);
    }
  }

  return moves;
}

DbState::DbState(std::shared_ptr<const Game> game) : State(game), game_(std::dynamic_pointer_cast<const DbGame>(game)) {}

std::string DbState::ActionToString(Player player,
                                    Action action_id) const {
  const auto &ca = game_->GetClientActions();
  const auto &sa = game_->GetServerActions();
  const std::string &sql = IsClient(player) ? ca[action_id]->GetIdentifier() : sa[action_id]->GetSQL();
  return absl::StrCat("Action(id=", action_id, ", player=", player, ", sql=", sql,  ")");
}

std::string DbState::ToString() const {
  std::stringstream output;
  output << "History[";
  for (const auto &pa : history_) {
    output << pa << ",";
  }
  output << "]";
  return output.str();
}

bool DbState::IsTerminal() const {
  return num_moves_ >= game_->MaxGameLength();
}

std::vector<double> DbState::Returns() const {
  double total_time = 0;

  pqxx::connection conn("host=127.0.0.1 port=5432 dbname=spiel user=spiel password=spiel sslmode=disable application_name=psql");
  pqxx::work txn(conn);

  const auto &client_actions = game_->GetClientActions();
  const auto &server_actions = game_->GetServerActions();
  for (const auto &player_action : history_) {
    const Action action = player_action.action;
    if (IsClient(player_action.player)) {
      pqxx::subtransaction subtxn(txn);
      // Execute the client query.
      Txn *c_txn = client_actions[action].get();
      double c_txn_cost = 0;
      for (const auto &sql: c_txn->GetSQL()) {
        std::string query = absl::StrCat("EXPLAIN (ANALYZE, BUFFERS) ", sql);
        pqxx::result rset{subtxn.exec(query)};
        TrueCost tc{&rset};
        total_time += (tc.actual_planning_time_ms_ + tc.actual_execution_time_ms_) * c_txn->GetWeight();
        c_txn_cost += tc.actual_planning_time_ms_ + tc.actual_execution_time_ms_;
      }
      std::cout << c_txn->GetIdentifier() << " took " << c_txn_cost << std::endl;
    } else {
      SPIEL_CHECK_TRUE(IsServer(player_action.player));
      std::string query = server_actions[action]->GetSQL();
      auto t1 = std::chrono::high_resolution_clock::now();
      pqxx::result rset{txn.exec0(query)};
      auto t2 = std::chrono::high_resolution_clock::now();
      double time_ms = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
      total_time += time_ms;
      std::cout << query << " took " << time_ms << std::endl;
    }
  }
  return {total_time, -total_time};
}

std::string DbState::InformationStateString(Player player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);
  return HistoryString();
}

std::string DbState::ObservationString(Player player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);
  return ToString();
}

void DbState::UndoAction(Player player, Action move) {
  current_player_ = player;
  num_moves_ -= 1;
  history_.pop_back();
  --move_number_;
}

std::unique_ptr<State> DbState::Clone() const {
  return std::unique_ptr<State>(new DbState(*this));
}

DbGame::DbGame(const GameParameters& params)
    : Game(kGameType, params) {
  client_.emplace_back(std::make_unique<TPCCNewOrder>());
  client_.emplace_back(std::make_unique<TPCCPayment>());
  client_.emplace_back(std::make_unique<TPCCOrderStatus>());
  client_.emplace_back(std::make_unique<TPCCDelivery>());
  client_.emplace_back(std::make_unique<TPCCStockLevel>());

  // These two indexes should be created by TPC-C.
  server_.emplace_back(std::make_unique<TuningAction>("CREATE INDEX IF NOT EXISTS idx_customer_name ON customer (c_w_id, c_d_id, c_last, c_first);"));
  server_.emplace_back(std::make_unique<TuningAction>("CREATE INDEX IF NOT EXISTS idx_order ON oorder (o_w_id, o_d_id, o_c_id, o_id);"));
  // Random bullshit.
  server_.emplace_back(std::make_unique<TuningAction>("CREATE INDEX IF NOT EXISTS garbage_1 ON oorder (o_w_id, o_d_id);"));
  server_.emplace_back(std::make_unique<TuningAction>("CREATE INDEX IF NOT EXISTS garbage_2 ON foo (a);"));
}

}  // namespace db
}  // namespace open_spiel
