#pragma once
#include "distrubution_strategy.h"
#include "input_parser.h"
#include "output_parser.h"
#include "pre_deal.h"

class Distributor {
 public:

  Distributor(int model = 1) : model_(model){};
  void DayDistribute();

 private:
  std::unordered_map<std::string, EdgeNode *> edge_node_;
  std::unordered_map<std::string, ClientNode *> client_node_;
  std::vector<two_string_key_int> day_distribution_;
  std::vector<std::unordered_set<std::string> > available_edge_node_;
  int model_;
  int day_;
};