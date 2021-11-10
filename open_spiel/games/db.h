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

#ifndef OPEN_SPIEL_GAMES_DB_H_
#define OPEN_SPIEL_GAMES_DB_H_

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include "open_spiel/spiel.h"

// Simple game of Noughts and Crosses:
// https://en.wikipedia.org/wiki/Tic-tac-toe
//
// Parameters: none

namespace open_spiel {
namespace db {

class DbGame;

// State of an in-play game.
class DbState : public State {
 public:
  DbState(std::shared_ptr<const Game> game);

  DbState(const DbState&) = default;
  DbState& operator=(const DbState&) = default;

  Player CurrentPlayer() const override {
    return IsTerminal() ? kTerminalPlayerId : current_player_;
  }
  std::string ActionToString(Player player, Action action_id) const override;
  std::string ToString() const override;
  bool IsTerminal() const override;
  std::vector<double> Returns() const override;
  std::string InformationStateString(Player player) const override;
  std::string ObservationString(Player player) const override;
  std::unique_ptr<State> Clone() const override;
  void UndoAction(Player player, Action move) override;
  std::vector<Action> LegalActions() const override;

 protected:
  void DoApplyAction(Action move) override;

 private:
  Player current_player_ = 0;         // Player zero goes first
  int num_moves_ = 0;

  std::shared_ptr<const DbGame> game_;
  std::set<Action> server_actions_;
};

class Txn {
 public:
  virtual const std::string &GetSQL() = 0;
};

class SingleQueryTxn : public Txn {
 public:
  SingleQueryTxn(std::string sql) : sql_(std::move(sql)) {}
  const std::string &GetSQL() override { return sql_; }
 private:
  const std::string sql_;
};

class TuningAction {
 public:
  TuningAction(std::string sql) : sql_(std::move(sql)) {}
  const std::string &GetSQL() const { return sql_; }
 private:
  const std::string sql_;
};

// Game object.
class DbGame : public Game {
 public:
  explicit DbGame(const GameParameters& params);
  int NumDistinctActions() const override { return std::max(client_.size(), server_.size()); }
  std::unique_ptr<State> NewInitialState() const override {
    return std::unique_ptr<State>(new DbState(shared_from_this()));
  }
  int NumPlayers() const override { return 2; }
  double MinUtility() const override { return -999999; }
  double UtilitySum() const override { return 0; }
  double MaxUtility() const override { return 999999; }
  int MaxGameLength() const override { return 6; }

  const std::vector<std::unique_ptr<Txn>> &GetClientActions() const { return client_; }
  const std::vector<std::unique_ptr<TuningAction>> &GetServerActions() const { return server_; }

 private:
  std::vector<std::unique_ptr<Txn>> client_;
  std::vector<std::unique_ptr<TuningAction>> server_;
};

}  // namespace db
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_DB_H_
