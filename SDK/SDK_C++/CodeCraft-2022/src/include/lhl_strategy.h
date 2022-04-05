#pragma once
#include "distribution_strategy.h"
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
#include <queue>
#include <algorithm>
#include <set>

class LHLStrategy : public DayDistribution{
  public:
    LHLStrategy(int days, Data *data) : DayDistribution(days, data){}

    void Distribute() override;
  private:
    void Init();
    void AverageStrategy();
    void BaselineStrategy();
    void ModifiedBaselineStrategy();
    std::vector<std::pair<std::string, int>> ApplyStreamsToEdge(std::vector<std::pair<std::string, int>> all_streams, std::string client, std::vector<std::string> edges);

    std::unordered_map<std::string, int> edge_node_remain_; 
};