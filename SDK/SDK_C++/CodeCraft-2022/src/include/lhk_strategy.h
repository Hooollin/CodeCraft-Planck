#pragma once
#include "distribution_strategy.h"
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

class LHKStrategy : public DayDistribution{
 public:
  LHKStrategy(int days, Data *data);

  void Distribute() override;

 private:

  void DistributeForCost();
  void DistributeBalanced();

  std::unordered_map<std::string, int> edge_node_remain_;
  std::vector<std::string> client_order_;
};