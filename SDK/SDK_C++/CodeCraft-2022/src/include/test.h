#pragma once
#include <map>
#include "client_node.h"
#include "distrubution_strategy.h"
#include "edge_node.h"
#include "input_parser.h"
#include "output_parser.h"
#include "pre_deal.h"
#include "data.h"


class Test {
 public:

  Test(int model) : model_(model) {data_ = *(new Data);}
  void TestEdgeAndClient();
  void TestAll();

 private:
  int model_;
  Data data_;
};