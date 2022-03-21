#pragma once
#include "distrubution_strategy.h"
#include "input_parser.h"
#include "output_parser.h"
#include "pre_deal.h"
#include "data.h"

class Distributor {
 public:

  Distributor(int model = 1) : model_(model){data_ = *(new Data);}

  void DayDistribute();

 private:
  Data data_;
  int model_;
};