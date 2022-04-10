#include "test.h"

void Test::TestAll() {
  //定义并处理处理输入
  InputParser input_parser(model_, &data_);
  input_parser.Parse();
  TestEdgeAndClient();

  //定义输出类
  OutputParser output_parser(model_, &data_);

  LHLPreDistribution pre_deal(&data_);
  pre_deal.Distribute();
  TestPreDeal();

  int allday = data_.GetAllDays();
  std::vector<int> days_order = data_.GetDaysOrder();

  for (int i = 0; i < allday; i++) {
    int nowaday = days_order[i];
    LHKStrategy day_strategy(nowaday, &data_);
    day_strategy.Distribute();
  }
  TestEverydaysDistribution();
  TestFinalCost();
  output_parser.StandradOutput();
}

Test::Test(int model) : model_(model) {
  data_ = *(new Data);
  assert(model >= 1 && model <= 3);
  if (model == 1) {
    client_and_edge_ = everydays_ = pre_deal_ = final_cost_ = online_pre_;
  } else if (model == 2) {
    client_and_edge_ = everydays_ = pre_deal_ = final_cost_ = linux_pre_;
  } else {
    client_and_edge_ = everydays_ = pre_deal_ = final_cost_ = windows_pre_;
  }
  client_and_edge_ += client_and_edge_suf_;
  everydays_ += everydays_suf_;
  final_cost_ += final_cost_suf_;
  pre_deal_ += pre_deal_suf_;
}

void Test::TestEdgeAndClient() {
  ofs_.clear();
  ofs_.open(client_and_edge_, std::ios::out);
  assert(ofs_.is_open());
  int days = data_.GetAllDays();
  std::vector<std::string> edge_node = data_.GetEdgeList();
  std::vector<std::string> client_node = data_.GetClientList();
  int edge_size = edge_node.size();
  int client_size = client_node.size();
  for (std::string edge : edge_node) {
    ofs_ << data_.EdgeToString(edge) << std::endl;
    assert(data_.GetEdgeBandwidthLimit(edge) >= 0);
    assert(data_.GetEdgeClientNum(edge) >= 0);
    assert(data_.GetEdgeClientNum(edge) <= client_size);
  }

  for (std::string client : client_node) {
    ClientNode *node = data_.GetClientNode(client);
    assert(node->GetDays() == days);
    for (int i = 0; i < days; i++) {
      std::unordered_map<std::string, int> stream =
          data_.GetClientDayTotalDemand(i, client);
      for (auto &p : stream) {
        assert(p.second >= 0);
      }
    }
    assert(data_.GetClientEdgeNum(client) >= 0 &&
           data_.GetClientEdgeNum(client) <= edge_size);
    ofs_ << node->ToString() << std::endl;
  }
  ofs_.close();
}

void Test::TestPreDeal() {
  ofs_.clear();
  ofs_.open(pre_deal_, std::ios::out);
  assert(ofs_.is_open());

  int allday = data_.GetAllDays();
  for (int i = 0; i < allday; i++) {
    ofs_ << "day" << i << ":" << std::endl;
    auto &distribution = data_.GetDistribution(i);
    //检测边缘节点使用的流量是否超过上限
    std::unordered_map<std::string, long long> edge_used_bandwidth;
    //输出可用节点
    std::unordered_set<std::string> available_edge_node =
        data_.GetAvailableEdgeNode(i);
    ofs_ << "can use available edge node :";
    for (std::string p : available_edge_node) ofs_ << p << " ";
    ofs_ << std::endl;
    for (auto &p : distribution) {
      std::string client = p.first;
      //检查分配是否是不存在的流分配
      std::unordered_map<std::string, int> demand =
          data_.GetClientDayTotalDemand(i, client);
      ofs_ << client << ":";
      //用来合并属于同一个边缘节点的流
      std::unordered_map<std::string, std::unordered_set<std::string> >
          sdistribution;
      for (auto &pp : p.second) {
        std::string stream = pp.first;
        std::string edge = pp.second;
        assert(demand.find(stream) != demand.end());
        assert(available_edge_node.find(edge) == available_edge_node.end());
        edge_used_bandwidth[edge] +=
            data_.GetClientStreamDemand(i, client, stream);
        sdistribution[edge].emplace(stream);
      }
      auto &remain_demand = data_.GetClientDayRemainingDemand(i, client);
      for (auto &pp : remain_demand) {
        assert(demand.find(pp.first) != demand.end());
      }
      for (auto &pp : demand) {
        assert(remain_demand.find(pp.first) != remain_demand.end() ||
               p.second.find(pp.first) != p.second.end());
      }
      //输出流
      for (auto &pp : sdistribution) {
        std::string edge = pp.first;
        ofs_ << "<" << edge << ",";
        for (std::string stream : pp.second) {
          ofs_ << stream << ",";
        }
        ofs_ << ">,";
      }
      ofs_ << std::endl;
    }
    //边缘节点上限检查
    for (auto p : edge_used_bandwidth) {
      std::string edge = p.first;
      long long cost = p.second;
      assert(cost <= data_.GetEdgeBandwidthLimit(edge));
    }
  }
  ofs_.close();
}

void Test::TestEverydaysDistribution() {
  ofs_.clear();
  ofs_.open(everydays_, std::ios::out);
  assert(ofs_.is_open());

  int allday = data_.GetAllDays();
  for (int i = 0; i < allday; i++) {
    ofs_ << "day" << i << ":" << std::endl;
    auto &distribution = data_.GetDistribution(i);
    //检测边缘节点使用的流量是否超过上限
    std::unordered_map<std::string, long long> edge_used_bandwidth;
    long long all_bandwidth = 0;
    for (auto &p : distribution) {
      std::string client = p.first;
      //检查是否全部都已经分配到distribution
      std::unordered_map<std::string, int> demand =
          data_.GetClientDayTotalDemand(i, client);
      ofs_ << client << ":";
      //用来合并属于同一个边缘节点的流
      std::unordered_map<std::string, std::unordered_set<std::string> >
          sdistribution;
      for (auto &pp : p.second) {
        std::string stream = pp.first;
        std::string edge = pp.second;
        assert(demand.find(stream) != demand.end());
        edge_used_bandwidth[edge] +=
            data_.GetClientStreamDemand(i, client, stream);
        all_bandwidth += data_.GetClientStreamDemand(i, client, stream);
        sdistribution[edge].emplace(stream);
      }
      auto &remain_demand = data_.GetClientDayRemainingDemand(i, client);
      for (auto &pp : remain_demand) {
        assert(pp.second == 0);
      }
      for (auto &pp : demand) {
        assert(remain_demand.find(pp.first) != remain_demand.end() ||
               p.second.find(pp.first) != p.second.end());
      }
      //输出流
      for (auto &pp : sdistribution) {
        std::string edge = pp.first;
        ofs_ << "<" << edge << ",";
        for (std::string stream : pp.second) {
          ofs_ << stream << ",";
        }
        ofs_ << ">,";
      }
      ofs_ << std::endl;
    }
    //边缘节点上限检查,输出边缘节点使用流量占比
    ofs_ << "总流量需求量: " << all_bandwidth << std::endl;
    ofs_ << "边缘节点流量使用情况 (边缘节点名，使用量/上限，节点使用率， "
         << "节点流量占总流量百分比) ：" << std::endl;
    std::vector<std::string> edge_node = data_.GetEdgeList();
    sort(edge_node.begin(), edge_node.end(),
         [&](std::string &a, std::string &b) {
           return edge_used_bandwidth[a] > edge_used_bandwidth[b];
         });
    for (std::string edge : edge_node) {
      long long cost = edge_used_bandwidth[edge];
      if (cost == 0) continue;
      long long up = data_.GetEdgeBandwidthLimit(edge);
      assert(cost <= data_.GetEdgeBandwidthLimit(edge));
      ofs_ << "(" << edge << "," << cost << "/" << up << "," << std::fixed
           << std::setprecision(2) << 100.0 * cost / up << "%," << std::fixed
           << std::setprecision(2) << 100.0 * cost / all_bandwidth << "%)"
           << std::endl;
    }
  }
  ofs_.close();
}

void Test::TestFinalCost() {
  ofs_.clear();
  ofs_.open(final_cost_, std::ios::out);
  assert(ofs_.is_open());

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
  ofs_ << "边缘节点95%百分位值 :" << std::endl;
  ofs_ << std::endl;
  long long total_cost = 0;
  for (auto &p : edge_days_bandwidth) {
    std::string edge = p.first;
    std::vector<int> &bandwidth = p.second;
    sort(bandwidth.begin(), bandwidth.end());
    int cost = bandwidth[percent_95 - 1];
    ofs_ << "<" << edge << "," << cost << ">" << std::endl;
    total_cost += cost;
  }
  ofs_ << std::endl;
  ofs_ << "总计边缘节点95%百分位值之和:" << total_cost << std::endl;
  ofs_ << std::endl;
  ofs_ << "边缘节点成本 :" << std::endl;
  ofs_ << std::endl;
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
    ofs_ << "<" << edge << "," << std::fixed << std::setprecision(2)
         << edge_cost << ">" << std::endl;
    total += edge_cost;
  }
  ofs_ << std::endl;
  ofs_ << "边缘节点成本总和:" << (long long)(total + 0.5 + 1e-6) << std::endl;
  ofs_ << std::endl;
  ofs_.close();
}

long long Test::CalFinalCost(){
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