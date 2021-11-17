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
  Player current_player_ = 1;         // Player zero (Client) goes first
  bool finished_ = false;

  int num_server_actions_ = 0;
  int num_client_actions_ = 0;
  int num_server_actions_this_turn_ = 0;
  int num_client_actions_this_turn_ = 0;

  std::unordered_set<size_t> client_actions_forcer_;

  std::shared_ptr<const DbGame> game_;
  std::set<Action> server_actions_;
};

class Txn {
 public:
  Txn(std::string identifier, double weight) : identifier_(identifier), weight_(weight) {}
  const std::string &GetIdentifier() const { return identifier_; }
  const std::vector<std::string> &GetSQL() const { return sql_; }
  const double GetWeight() const { return weight_; }
 protected:
  std::string identifier_;
  std::vector<std::string> sql_;
  const double weight_;
};

class SingleQueryTxn : public Txn {
 public:
  SingleQueryTxn(std::string sql, double weight=1) : Txn(sql, weight) {
    sql_.emplace_back(std::move(sql));
  }
};

class TPCCNewOrder : public Txn {
 public:
  TPCCNewOrder(double weight=45) : Txn("NewOrder", weight) {
    sql_.emplace_back("SELECT C_DISCOUNT, C_LAST, C_CREDIT  FROM customer WHERE C_W_ID = 1    AND C_D_ID = 7    AND C_ID = 671");
    sql_.emplace_back("SELECT W_TAX   FROM warehouse WHERE W_ID = 1");
    sql_.emplace_back("SELECT D_NEXT_O_ID, D_TAX   FROM district WHERE D_W_ID = 1 AND D_ID = 7 FOR UPDATE");
    sql_.emplace_back("UPDATE district   SET D_NEXT_O_ID = D_NEXT_O_ID + 1  WHERE D_W_ID = 1    AND D_ID = 7");
    sql_.emplace_back("INSERT INTO oorder (O_ID, O_D_ID, O_W_ID, O_C_ID, O_ENTRY_D, O_OL_CNT, O_ALL_LOCAL) VALUES (4303, 7, 1, 671, '2021-11-10 18:10:06.637-05', 15, 1)");
    sql_.emplace_back("INSERT INTO new_order (NO_O_ID, NO_D_ID, NO_W_ID)  VALUES ( 4303, 7, 1)");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 15399");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 15399    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 20183");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 20183    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 47907");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 47907    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 44247");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 44247    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 88231");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 88231    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 71143");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 71143    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 88674");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 88674    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 24166");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 24166    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 81315");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 81315    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 32473");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 32473    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 65158");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 65158    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 94949");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 94949    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 15814");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 15814    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 15975");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 15975    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("SELECT I_PRICE, I_NAME , I_DATA   FROM item WHERE I_ID = 38374");
    sql_.emplace_back("SELECT S_QUANTITY, S_DATA, S_DIST_01, S_DIST_02, S_DIST_03, S_DIST_04, S_DIST_05,        S_DIST_06, S_DIST_07, S_DIST_08, S_DIST_09, S_DIST_10  FROM stock WHERE S_I_ID = 38374    AND S_W_ID = 1 FOR UPDATE");
    sql_.emplace_back("INSERT INTO order_line (OL_O_ID, OL_D_ID, OL_W_ID, OL_NUMBER, OL_I_ID, OL_SUPPLY_W_ID, OL_QUANTITY, OL_AMOUNT, OL_DIST_INFO)  VALUES (4303,7,1,15,38374,1,8,768.0,'gygnrukbdbhfeiohzplgxvr ')");
    sql_.emplace_back("UPDATE stock   SET S_QUANTITY = 57 ,        S_YTD = S_YTD + 8,        S_ORDER_CNT = S_ORDER_CNT + 1,        S_REMOTE_CNT = S_REMOTE_CNT + 0  WHERE S_I_ID = 38374    AND S_W_ID = 1");
  }
};

class TPCCPayment : public Txn {
 public:
  TPCCPayment(double weight=43) : Txn("Payment", weight) {
    sql_.emplace_back("UPDATE warehouse   SET W_YTD = W_YTD + '874.0'::numeric  WHERE W_ID = 1");
    sql_.emplace_back("SELECT W_STREET_1, W_STREET_2, W_CITY, W_STATE, W_ZIP, W_NAME  FROM warehouse WHERE W_ID = 1");
    sql_.emplace_back("UPDATE district   SET D_YTD = D_YTD + '874.0'::numeric  WHERE D_W_ID = 1    AND D_ID = 7");
    sql_.emplace_back("SELECT D_STREET_1, D_STREET_2, D_CITY, D_STATE, D_ZIP, D_NAME  FROM district WHERE D_W_ID = 1    AND D_ID = 7");
    sql_.emplace_back("SELECT C_FIRST, C_MIDDLE, C_ID, C_STREET_1, C_STREET_2, C_CITY,        C_STATE, C_ZIP, C_PHONE, C_CREDIT, C_CREDIT_LIM, C_DISCOUNT,        C_BALANCE, C_YTD_PAYMENT, C_PAYMENT_CNT, C_SINCE   FROM customer WHERE C_W_ID = 1    AND C_D_ID = 7    AND C_LAST = 'ANTIATIONANTI'  ORDER BY C_FIRST");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = -9299.669921875,        C_YTD_PAYMENT = 9299.669921875,        C_PAYMENT_CNT = 4  WHERE C_W_ID = 1    AND C_D_ID = 7    AND C_ID = 687");
    sql_.emplace_back("INSERT INTO history (H_C_D_ID, H_C_W_ID, H_C_ID, H_D_ID, H_W_ID, H_DATE, H_AMOUNT, H_DATA)  VALUES (7,1,687,7,1,'2021-11-10 18:10:05.344-05',874.0,'kumroe    ckvezt')");
  }
};

class TPCCOrderStatus : public Txn {
 public:
  TPCCOrderStatus(double weight=4) : Txn("OrderStatus", weight) {
    sql_.emplace_back("SELECT C_FIRST, C_MIDDLE, C_LAST, C_STREET_1, C_STREET_2,        C_CITY, C_STATE, C_ZIP, C_PHONE, C_CREDIT, C_CREDIT_LIM,        C_DISCOUNT, C_BALANCE, C_YTD_PAYMENT, C_PAYMENT_CNT, C_SINCE   FROM customer WHERE C_W_ID = 1    AND C_D_ID = 3    AND C_ID = 728");
    sql_.emplace_back("SELECT O_ID, O_CARRIER_ID, O_ENTRY_D   FROM oorder WHERE O_W_ID = 1    AND O_D_ID = 3    AND O_C_ID = 728  ORDER BY O_ID DESC LIMIT 1");
    sql_.emplace_back("SELECT OL_I_ID, OL_SUPPLY_W_ID, OL_QUANTITY, OL_AMOUNT, OL_DELIVERY_D   FROM order_line WHERE OL_O_ID = 124   AND OL_D_ID = 3   AND OL_W_ID = 1");
  }
};

class TPCCDelivery : public Txn {
 public:
  TPCCDelivery(double weight=4) : Txn("Delivery", weight) {
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 1    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3234    AND NO_D_ID = 1   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3234    AND O_D_ID = 1    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3234    AND O_D_ID = 1   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3234    AND OL_D_ID = 1    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3234    AND OL_D_ID = 1    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '1158.93994140625'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 1    AND C_ID = 312");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 2    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3226    AND NO_D_ID = 2   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3226    AND O_D_ID = 2    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3226    AND O_D_ID = 2   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3226    AND OL_D_ID = 2    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3226    AND OL_D_ID = 2    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '3939.3798828125'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 2    AND C_ID = 1793");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 3    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3222    AND NO_D_ID = 3   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3222    AND O_D_ID = 3    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3222    AND O_D_ID = 3   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3222    AND OL_D_ID = 3    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3222    AND OL_D_ID = 3    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '3056.97998046875'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 3    AND C_ID = 2851");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 4    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3225    AND NO_D_ID = 4   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3225    AND O_D_ID = 4    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3225    AND O_D_ID = 4   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3225    AND OL_D_ID = 4    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3225    AND OL_D_ID = 4    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '1563.9200439453125'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 4    AND C_ID = 1131");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 5    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3222    AND NO_D_ID = 5   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3222    AND O_D_ID = 5    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3222    AND O_D_ID = 5   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3222    AND OL_D_ID = 5    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3222    AND OL_D_ID = 5    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '2561.52001953125'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 5    AND C_ID = 1918");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 6    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3220    AND NO_D_ID = 6   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3220    AND O_D_ID = 6    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3220    AND O_D_ID = 6   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3220    AND OL_D_ID = 6    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3220    AND OL_D_ID = 6    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '3418.989990234375'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 6    AND C_ID = 74");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 7    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3215    AND NO_D_ID = 7   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3215    AND O_D_ID = 7    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3215    AND O_D_ID = 7   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3215    AND OL_D_ID = 7    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3215    AND OL_D_ID = 7    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '3089.179931640625'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 7    AND C_ID = 314");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 8    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3215    AND NO_D_ID = 8   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3215    AND O_D_ID = 8    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3215    AND O_D_ID = 8   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3215    AND OL_D_ID = 8    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3215    AND OL_D_ID = 8    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '3846.3798828125'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 8    AND C_ID = 891");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 9    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3221    AND NO_D_ID = 9   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3221    AND O_D_ID = 9    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3221    AND O_D_ID = 9   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3221    AND OL_D_ID = 9    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3221    AND OL_D_ID = 9    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '2053.89990234375'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 9    AND C_ID = 883");
    sql_.emplace_back("SELECT NO_O_ID FROM new_order WHERE NO_D_ID = 10    AND NO_W_ID = 1  ORDER BY NO_O_ID ASC  LIMIT 1");
    sql_.emplace_back("DELETE FROM new_order WHERE NO_O_ID = 3214    AND NO_D_ID = 10   AND NO_W_ID = 1");
    sql_.emplace_back("SELECT O_C_ID FROM oorder WHERE O_ID = 3214    AND O_D_ID = 10    AND O_W_ID = 1");
    sql_.emplace_back("UPDATE oorder   SET O_CARRIER_ID = 5  WHERE O_ID = 3214    AND O_D_ID = 10   AND O_W_ID = 1");
    sql_.emplace_back("UPDATE order_line   SET OL_DELIVERY_D = '2021-11-10 18:10:06.834-05'  WHERE OL_O_ID = 3214    AND OL_D_ID = 10    AND OL_W_ID = 1");
    sql_.emplace_back("SELECT SUM(OL_AMOUNT) AS OL_TOTAL   FROM order_line WHERE OL_O_ID = 3214    AND OL_D_ID = 10    AND OL_W_ID = 1");
    sql_.emplace_back("UPDATE customer   SET C_BALANCE = C_BALANCE + '2218.110107421875'::numeric,       C_DELIVERY_CNT = C_DELIVERY_CNT + 1  WHERE C_W_ID = 1    AND C_D_ID = 10    AND C_ID = 617");
  }
};

class TPCCStockLevel : public Txn {
 public:
  TPCCStockLevel(double weight=4) : Txn("StockLevel", weight) {
    sql_.emplace_back("SELECT D_NEXT_O_ID   FROM district WHERE D_W_ID = 1    AND D_ID = 8");
    sql_.emplace_back("SELECT COUNT(DISTINCT (S_I_ID)) AS STOCK_COUNT  FROM order_line, stock WHERE OL_W_ID = 1 AND OL_D_ID = 8 AND OL_O_ID < 4254 AND OL_O_ID >= 4234 AND S_W_ID = 1 AND S_I_ID = OL_I_ID AND S_QUANTITY < 11");
  }
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
  double MinUtility() const override { return -100000; }
  double UtilitySum() const override { return 0; }
  double MaxUtility() const override { return 100000; }
  int MaxGameLength() const override { return MaxServerMoves() * MaxClientMovesPerTurn() * MaxServerMovesPerTurn(); }

  int MaxClientMovesPerTurn() const { return 12; }
  int MaxServerMovesPerTurn() const { return 1; }
  int MaxServerMoves() const { return 2; }
  bool UseRealCost() const { return true; }

  const std::vector<std::unique_ptr<Txn>> &GetClientActions() const { return client_; }
  const std::vector<std::unique_ptr<TuningAction>> &GetServerActions() const { return server_; }

 private:
  std::vector<std::unique_ptr<Txn>> client_;
  std::vector<std::unique_ptr<TuningAction>> server_;
};

}  // namespace db
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_DB_H_
