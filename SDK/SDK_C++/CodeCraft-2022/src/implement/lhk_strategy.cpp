#include "lhk_strategy.h"

void LHKStrategy::Distribute() {
  DistributeForCost();
  DistributeForBaseCost();
  DistributeBalanced();
}

LHKStrategy::LHKStrategy(int days, Data *data) : DayDistribution(days, data) {
  // 初始化边缘节点的剩余量
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  for (auto edge : available_edge_node) {
    edge_node_remain_[edge] = data_->GetEdgeBandwidthLimit(edge);
  }
  //获取客户节点一天的请求总量，以及连边数,以初始化客户节点遍历顺序
  client_order_ = data_->GetClientList();
  std::unordered_map<std::string, long long> client_demand;
  std::unordered_map<std::string, int> connect_number;
  for (std::string &client : client_order_) {
    std::unordered_map<std::string, int> demand =
        data_->GetClientDayRemainingDemand(days, client);
    for (auto &p : demand) {
      client_demand[client] += p.second;
    }
    std::unordered_set<std::string> connected_edge =
        data_->GetClientEdge(client);
    for (std::string edge : connected_edge) {
      if (available_edge_node.find(edge) != available_edge_node.end()) {
        connect_number[client]++;
      }
    }
  }
  //按照连接数为第一优先级从小到大，流量和为第二优先级从大到小获得客户节点遍历顺序
  std::sort(client_order_.begin(), client_order_.end(),
            [&](std::string &a, std::string &b) {
              return connect_number[a] == connect_number[b]
                         ? client_demand[a] > client_demand[b]
                         : connect_number[a] < connect_number[b];
            });
  return;
}

void LHKStrategy::DistributeBalanced() {
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  for (std::string &client : client_order_) {
    //该客户节点连接的可用边缘节点堆，堆顶是剩余流量最大的边缘节点
    std::priority_queue<std::pair<int, std::string>> edge_bandwidth_heap;
    std::unordered_set<std::string> connected_edge =
        data_->GetClientEdge(client);
    //堆中加入可用连接的边缘节点
    for (std::string edge : connected_edge) {
      if (available_edge_node.find(edge) != available_edge_node.end()) {
        if (edge_node_remain_[edge] > 0)
          edge_bandwidth_heap.emplace(edge_node_remain_[edge], edge);
      }
    }
    //对客户节点的流带宽需求从大到小排序,获得分配流的遍历顺序
    std::unordered_map<std::string, int> client_day_stream =
        data_->GetClientDayRemainingDemand(days_, client);
    std::vector<std::string> stream_order;
    for (auto &p : client_day_stream) {
      stream_order.emplace_back(p.first);
    }
    std::sort(stream_order.begin(), stream_order.end(),
              [&](const std::string &a, const std::string &b) {
                return client_day_stream[a] > client_day_stream[b];
              });
    //分配流
    for (std::string &stream : stream_order) {
      int demand_bandwidth = client_day_stream[stream];
      if (demand_bandwidth == 0) continue;
      assert(!edge_bandwidth_heap.empty());
      int edge_bandwidth = edge_bandwidth_heap.top().first;
      std::string edge = edge_bandwidth_heap.top().second;
      assert(demand_bandwidth <= edge_bandwidth);
      edge_bandwidth_heap.pop();
      data_->SetDistribution(days_, client, stream, edge);
      edge_node_remain_[edge] -= demand_bandwidth;
      if (edge_node_remain_[edge] > 0)
        edge_bandwidth_heap.emplace(edge_node_remain_[edge], edge);
    }
  }
  int base_cost = data_->GetBaseCost();
  //更新今日边缘节点成本
  for (auto &p : edge_node_remain_) {
    std::string edge = p.first;
    if (data_->GetEdgeBandwidthLimit(edge) == p.second) continue;
    int cost =
        std::max(data_->GetEdgeBandwidthLimit(edge) - p.second, base_cost);
//    int cost = data_->GetEdgeBandwidthLimit(edge) - p.second;
    data_->UpdateEdgeCost(edge, cost);
  }

  return;
}

void LHKStrategy::DistributeForCost() {
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  //每日剩余可用成本上限
  std::unordered_map<std::string, int> leave_cost;
  for (std::string edge : available_edge_node) {
    leave_cost[edge] =
        data_->GetEdgeCost(edge) -
        (data_->GetEdgeBandwidthLimit(edge) - edge_node_remain_[edge]);
  }
  for (std::string &client : client_order_) {
    //该客户节点连接的可用边缘节点堆，堆顶是剩余成本流量最大的边缘节点
    std::priority_queue<std::pair<int, std::string>> edge_bandwidth_heap;
    std::unordered_set<std::string> connected_edge =
        data_->GetClientEdge(client);
    //堆中加入可用连接的边缘节点
    for (std::string edge : connected_edge) {
      if (available_edge_node.find(edge) != available_edge_node.end()) {
        if (leave_cost[edge] > 0)
          edge_bandwidth_heap.emplace(leave_cost[edge], edge);
      }
    }
    //对客户节点的流带宽需求从大到小排序,获得分配流的遍历顺序
    std::unordered_map<std::string, int> client_day_stream =
        data_->GetClientDayRemainingDemand(days_, client);
    std::vector<std::string> stream_order;
    for (auto &p : client_day_stream) {
      stream_order.emplace_back(p.first);
    }
    std::sort(stream_order.begin(), stream_order.end(),
              [&](const std::string &a, const std::string &b) {
                return client_day_stream[a] > client_day_stream[b];
              });
    //分配流
    for (std::string &stream : stream_order) {
      int demand_bandwidth = client_day_stream[stream];
      if (demand_bandwidth == 0) continue;
      if (edge_bandwidth_heap.empty()) break;
      int edge_bandwidth = edge_bandwidth_heap.top().first;
      std::string edge = edge_bandwidth_heap.top().second;
      if (demand_bandwidth > edge_bandwidth) continue;
      edge_bandwidth_heap.pop();
      data_->SetDistribution(days_, client, stream, edge);
      leave_cost[edge] -= demand_bandwidth;
      if (leave_cost[edge] > 0)
        edge_bandwidth_heap.emplace(leave_cost[edge], edge);
    }
  }
}

void LHKStrategy::DistributeFormula() {
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  for (std::string &client : client_order_) {
    std::unordered_set<std::string> connected_edge =
        data_->GetClientEdge(client);
    std::vector<std::string> available_edge(0);
    //堆中加入可用连接的边缘节点
    for (std::string edge : connected_edge) {
      if (available_edge_node.find(edge) != available_edge_node.end()) {
        if (edge_node_remain_[edge] > 0) available_edge.emplace_back(edge);
      }
    }
    //对客户节点的流带宽需求从大到小排序,获得分配流的遍历顺序
    std::unordered_map<std::string, int> client_day_stream =
        data_->GetClientDayRemainingDemand(days_, client);
    std::vector<std::string> stream_order;
    for (auto &p : client_day_stream) {
      stream_order.emplace_back(p.first);
    }
    std::sort(stream_order.begin(), stream_order.end(),
              [&](const std::string &a, const std::string &b) {
                return client_day_stream[a] > client_day_stream[b];
              });
    //分配流
    for (std::string &stream : stream_order) {
      int demand_bandwidth = client_day_stream[stream];
      if (demand_bandwidth == 0) continue;
      //边缘节点的最小增量
      double min_inc = 1e30;
      std::string min_inc_edge;
      for (std::string &edge : available_edge) {
        long long bandwidth_limit = data_->GetEdgeBandwidthLimit(edge);
        long long current_load = bandwidth_limit - edge_node_remain_[edge];

        double base = std::max(0LL, current_load - data_->GetBaseCost());
        double current_cost =
            1.0 * base * base / bandwidth_limit +
            std::max(current_load, 1LL * data_->GetBaseCost());

        double after_base =
            std::max(0LL, current_load + client_day_stream[stream] -
                              data_->GetBaseCost());
        double after_cost = 1.0 * after_base * after_base / bandwidth_limit +
                            std::max(current_load + client_day_stream[stream],
                                     1LL * data_->GetBaseCost());
        double increment = after_cost - current_cost;
        if (increment < min_inc) {
          min_inc = increment;
          min_inc_edge = edge;
        }
      }
      data_->SetDistribution(days_, client, stream, min_inc_edge);
      edge_node_remain_[min_inc_edge] -= demand_bandwidth;
    }
  }
  //更新今日边缘节点成本
  int base_cost = data_->GetBaseCost();
  for (auto &p : edge_node_remain_) {
    std::string edge = p.first;
    if (data_->GetEdgeBandwidthLimit(edge) == p.second) continue;
    int cost =
        std::max(data_->GetEdgeBandwidthLimit(edge) - p.second, base_cost);
    data_->UpdateEdgeCost(edge, cost);
  }

  return;
}

bool LHKStrategy::DistributeBalancedStream() {
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  std::unordered_map<std::string, int> copy_edge_node_remain_ =
      edge_node_remain_;
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      distribution;

  std::vector<isspr> stream_order;
  for (std::string &client : client_order_) {
    std::unordered_map<std::string, int> streams =
        data_->GetClientDayRemainingDemand(days_, client);
    for (auto &p : streams) {
      stream_order.emplace_back(
          std::make_pair(p.second, std::make_pair(client, p.first)));
    }
  }
  std::sort(stream_order.begin(), stream_order.end(), std::greater<isspr>());
  for (auto &current_stream : stream_order) {
    int load = current_stream.first;
    std::string client = current_stream.second.first,
                stream_id = current_stream.second.second;
    std::unordered_set<std::string> connected_edge =
        data_->GetClientEdge(client);
    std::string max_remain_edge;
    int max_remain = -1;
    for (std::string edge : connected_edge) {
      if (available_edge_node.find(edge) == available_edge_node.end()) continue;
      if (copy_edge_node_remain_[edge] < load) continue;

      if (max_remain < copy_edge_node_remain_[edge]) {
        max_remain = copy_edge_node_remain_[edge];
        max_remain_edge = edge;
      }
    }
    if (max_remain == -1) {
      return false;
    }
    distribution[client][stream_id] = max_remain_edge;
    copy_edge_node_remain_[max_remain_edge] -= load;
  }
  for (auto &p : distribution) {
    std::string client = p.first;
    for (auto &pp : p.second) {
      std::string stream_id = pp.first, edge = pp.second;
      data_->SetDistribution(days_, client, stream_id, edge);
    }
  }
  edge_node_remain_ = std::move(copy_edge_node_remain_);
  return true;
}

void LHKStrategy::DistributeForBaseCost() {
  int base_cost = data_->GetBaseCost();
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  std::unordered_map<std::string, int> edge_leave_base_cost;
  std::vector<std::string> edge_list = data_->GetEdgeList();
  for (std::string &edge : edge_list) {
    if (edge_node_remain_.find(edge) != edge_node_remain_.end() &&
        edge_node_remain_[edge] != data_->GetEdgeBandwidthLimit(edge)) {
      available_edge_node.erase(edge);
    }
  }
  for (std::string edge : available_edge_node)
    edge_leave_base_cost[edge] = base_cost;
  for (std::string &client : client_order_) {
    std::unordered_set<std::string> connected_edge =
        data_->GetClientEdge(client);
    std::vector<std::string> edge_order;
    for (std::string edge : connected_edge) {
      if (available_edge_node.find(edge) != available_edge_node.end())
        edge_order.emplace_back(edge);
    }
    std::sort(edge_order.begin(), edge_order.end(),
              [&](std::string &a, std::string &b) {
                return edge_leave_base_cost[a] == edge_leave_base_cost[b]
                           ? data_->GetEdgeBandwidthLimit(a) >
                                 data_->GetEdgeBandwidthLimit(b)
                           : edge_leave_base_cost[a] < edge_leave_base_cost[b];
              });
    //对客户节点的流带宽需求从大到小排序,获得分配流的遍历顺序
    std::unordered_map<std::string, int> client_day_stream =
        data_->GetClientDayRemainingDemand(days_, client);
    std::vector<std::string> stream_order;
    for (auto &p : client_day_stream) {
      stream_order.emplace_back(p.first);
    }
    std::sort(stream_order.begin(), stream_order.end(),
              [&](const std::string &a, const std::string &b) {
                return client_day_stream[a] > client_day_stream[b];
              });
    //分配流
    for (std::string &stream : stream_order) {
      int demand_bandwidth = client_day_stream[stream];
      if (demand_bandwidth == 0) continue;
      int min_leave = INT_MAX;
      std::string min_edge = "";
      for(std::string &edge : edge_order){
        if(edge_leave_base_cost[edge] >= demand_bandwidth){
          if(edge_leave_base_cost[edge] - demand_bandwidth < min_leave){
            min_leave = edge_leave_base_cost[edge] - demand_bandwidth;
            min_edge = edge;
          }
        }
      }
      if(min_leave == INT_MAX) continue;
      data_->SetDistribution(days_,client,stream,min_edge);
      edge_leave_base_cost[min_edge] -= demand_bandwidth;
    }
  }
}