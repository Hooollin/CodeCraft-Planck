#pragma once
#include <map>
#include "client_node.h"
#include "distrubution_strategy.h"
#include "edge_node.h"
#include "input_parser.h"
#include "output_parser.h"
#include "pre_deal.h"

class Test {
 public:
  Test(int model) : model_(model) {}

  void TestClientDistribution();
  void TestEdgeDistribution();

  void OutputTwoStringKeyInt(two_string_key_int &data);
 private:
  int model_;
};