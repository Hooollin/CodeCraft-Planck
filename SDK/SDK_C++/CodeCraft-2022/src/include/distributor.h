#pragma once
#include "data.h"
#include "input_parser.h"
#include "lhl_pre_deal.h"
#include "lhl_strategy.h"
#include "output_parser.h"
#include "lhk_strategy.h"
#include "lhl_pre_deal.h"

class Distributor {
 public:
  Distributor(int model = 1) : model_(model) {}

  void DayDistribute();
  long long CalFinalCost();

 private:
  Data data_;
  int model_;
};