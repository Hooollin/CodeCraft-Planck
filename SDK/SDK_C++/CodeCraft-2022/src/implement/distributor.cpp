
#include "distributor.h"
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

void Distributor::DayDistribute() {
  //定义并处理处理输入
  InputParser input_parser(model_, &data_);
  input_parser.Parse();
  //定义输出类
  OutputParser output_parser(model_, &data_);
  //预处理
  // LHLPreDistribution pre_distribution(&data_);
  // pre_distribution.Distribute();
  LHLPreDistribution pre_distribution(&data_);
  pre_distribution.Distribute();
  int allday = data_.GetAllDays();

  std::vector<int> days_order = data_.GetDaysOrder();

  for (int i = 0; i < allday; i++) {
    int nowaday = days_order[i];
    LHKStrategy day_strategy(nowaday, &data_);
    day_strategy.Distribute();
  }

  output_parser.StandradOutput();
  return;
}

long long Distributor::CalFinalCost() {
  int alldays = data_.GetAllDays();
  int percent_95 = (0.95 * alldays) - (int)(0.95 * alldays) > 1e-6
                       ? (0.95 * alldays + 1)
                       : (0.95 * alldays);
  std::unordered_map<std::string, std::vector<int> > edge_days_bandwidth;
  std::vector<std::string> edge_node = data_.GetEdgeList();
  for (std::string edge : edge_node) {
    edge_days_bandwidth[edge].clear();
    edge_days_bandwidth[edge].resize(alldays);
  }
  for (int i = 0; i < alldays; i++) {
    auto distribution = data_.GetDistribution(i);
    for (auto &p : distribution) {
      std::string client = p.first;
      for (auto &pp : p.second) {
        std::string edge = pp.second;
        std::string stream = pp.first;
        edge_days_bandwidth[edge][i] +=
            data_.GetClientStreamDemand(i, client, stream);
      }
    }
  }
  long long base_cost = data_.GetBaseCost();
  double total = 0;
  for (auto &p : edge_days_bandwidth) {
    std::string edge = p.first;
    std::vector<int> &bandwidth = p.second;
    sort(bandwidth.begin(), bandwidth.end());
    int bd = bandwidth[percent_95 - 1];
    double edge_cost = 0;
    if (bd == 0)
      edge_cost = 0;
    else if (bd <= base_cost)
      edge_cost = base_cost;
    else {
      edge_cost = 1.0 * (bd - base_cost) * (bd - base_cost) /
                      data_.GetEdgeBandwidthLimit(edge) +
                  bd;
    }
    total += edge_cost;
  }
  return (long long)(total + 0.5 + 1e-6);
}