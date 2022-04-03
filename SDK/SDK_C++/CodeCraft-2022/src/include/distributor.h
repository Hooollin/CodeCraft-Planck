#pragma once
#include "client_day_distribution.h"
#include "data.h"
#include "distribution_strategy.h"
#include "input_parser.h"
#include "output_parser.h"
#include "pre_deal_lhk.h"
#include "lhl_strategy.h"

class Distributor {
 public:
  Distributor(int model = 1) : model_(model) {}

  void DayDistribute();

 private:
  Data data_;
  int model_;
};