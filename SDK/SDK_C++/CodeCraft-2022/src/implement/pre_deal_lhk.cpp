
#include "pre_deal_lhk.h"

void LHKPreDistribution::Distribute() {
  // LHKDistribute();
  LHLDistribute();

  // update data
  GetEdgeOrder();
  GetClientOrder();
  GetDaysOrder();
}

int LHKPreDistribution::GetAvangeBandwidth(
    std::vector<std::string> &client_lists, int &bandwidth, int &deal_day) {
  int n = client_lists.size();
  int l = 0;
  int r = days_client_bandwidth_[deal_day][client_lists[n - 1]];
  int res = 0;
  while (l <= r) {
    int mid = (l + r) >> 1;
    long long total = 0;
    for (std::string &client : client_lists)
      total += std::min(days_client_bandwidth_[deal_day][client], mid);
    if (total <= bandwidth) {
      res = mid;
      l = mid + 1;
    } else {
      r = mid - 1;
    }
  }
  return res;
}

void LHKPreDistribution::LHLDistribute() {
  //前5%的天数
  int percent_five_day = std::max((int)(1.0 * (data_->GetAllDays()) * 0.05), 0);

  std::vector<std::string> available_edge_node = edge_node_;
  int n = available_edge_node.size();
  int l = allday_;

  //客户机连边数
  std::unordered_map<std::string, int> client_edge_num;
  for (std::string &client : client_node_) {
    client_edge_num[client] = data_->GetClientEdgeNum(client);
  }
  //获得每日流量需求
  days_client_bandwidth_ = std::vector<std::unordered_map<std::string, int>>(l);
  //每日客户节点连接边缘节点已被处理数
  std::vector<std::unordered_map<std::string, int>> days_client_deal_nums(l);
  for (int i = 0; i < allday_; i++) {
    for (std::string &client : client_node_) {
      auto streams = data_->GetClientDayRemainingDemand(i, client);
      int demand = 0;
      for(auto it = streams.begin(); it != streams.end(); ++it){
        demand += it->second;
      }
      days_client_bandwidth_[i][client] = demand;
      assert(demand >= 0);
    }
  }
  //每日使用的边缘节点列表
  std::vector<std::vector<std::string>> used_edge(l,
                                                  std::vector<std::string>(0));

  //获得每日的95%节点集合，但是没有真正的分配
  for (int i = 0; i < n; i++) {
    long long max_perfent_five = 0, max_bandwidth = 0,
              max_serving_client_count = 0;
    //每个边缘节点前5%大的天数的流量总和
    std::unordered_map<std::string, std::vector<long long>> percent_five{};

    //每个边缘节点前5%大的天数
    std::unordered_map<std::string, std::vector<int>> percent_days{};

    for (int j = 0; j < n; j++) {
      percent_five[available_edge_node[j]] = std::vector<long long>(3, 0);
      percent_five[available_edge_node[j]][1] =
          data_->GetEdgeNode(available_edge_node[j])->GetBandwidth();
      percent_five[available_edge_node[j]][2] =
          data_->GetEdgeNode(available_edge_node[j])->GetservingclientnodeNum();
      max_bandwidth =
          std::max(percent_five[available_edge_node[j]][1], max_bandwidth);
      max_serving_client_count = std::max(
          percent_five[available_edge_node[j]][2], max_serving_client_count);
      percent_days[available_edge_node[j]] = std::vector<int>(0);
    }
    //统计当前剩下的n-i个边缘节点的每日带宽
    for (int j = 0; j < n - i; j++) {
      std::string edge = available_edge_node[j];

      std::vector<long long> edge_bandwidth(l, 0);

      std::vector<int> seq(l);

      for (int k = 0; k < l; k++) seq[k] = k;

      std::unordered_set<std::string> edge_client = data_->GetEdgeClient(edge);
      for (int k = 0; k < l; k++) {
        for (auto &client : edge_client) {
          edge_bandwidth[k] += days_client_bandwidth_[k][client];
          assert(days_client_bandwidth_[k][client] >= 0);
          assert(edge_bandwidth[k] >= 0);
        }
      }

      std::sort(seq.begin(), seq.end(), [&](const int &a, const int &b) {
        return edge_bandwidth[a] > edge_bandwidth[b];
      });

      long long total = 0;
      for (int k = 0; k < percent_five_day; k++) {
        total += edge_bandwidth[seq[k]];
        percent_days[edge].emplace_back(seq[k]);
      }
      percent_five[edge][0] = total;
      max_perfent_five = std::max(total, max_perfent_five);
    }
    // 310w: 0.01, 1.0, 0.15
    // 300w: 0.15, 1.0, 0.1
    // 300w: 0.05, 1.0, 0.1
    // 304w: 0.02, 1.0, 0.1
    // 298w: 0.01, 1.0, 0.1 (minus)
    // loading : bandwidth : servingcount
    double w1 = 0.01, w2 = 1.0, w3 = 0.1;
    std::unordered_map<std::string, double> f;
    for (std::string edge : available_edge_node) {
      f[edge] = w1 * percent_five[edge][0] / max_perfent_five +
                w2 * percent_five[edge][1] / max_bandwidth -
                w3 * percent_five[edge][2] / max_serving_client_count;
    }
    //对边缘节点按照带宽最大5%排序
    sort(available_edge_node.begin(), available_edge_node.end(),
         [&](const std::string &a, const std::string &b) {
           return f[a] < f[b];
         });
    //取最大的边缘节点
    std::string max_edge = available_edge_node[available_edge_node.size() - 1];
    available_edge_node.resize(n - i - 1);

    std::unordered_set<std::string> &edge_client =
        data_->GetEdgeClient(max_edge);
    std::vector<std::string> client_lists;
    //按照客户端连边数对客户节点排序
    for (std::string client : edge_client) client_lists.emplace_back(client);
    sort(client_lists.begin(), client_lists.end(),
         [&](const std::string &a, const std::string &b) {
           return client_edge_num[a] < client_edge_num[b];
         });
    
    //对每天做处理
    for (int deal_day : percent_days[max_edge]) {
      //剩余流量
      int leave_bandwidth = data_->GetEdgeBandwidthLimit(max_edge);

      assert(leave_bandwidth >= 0);

      //按照客户端节点连边数从小到大更新
      for (std::string &client : client_lists) {
        if (leave_bandwidth == 0) break;
        
        auto &streams = data_->GetClientDayRemainingDemand(deal_day, client);
        for(auto &stream : streams){
          std::string stream_id = stream.first;
          int stream_cost = stream.second;

          if(stream_cost == 0) continue;

          if (leave_bandwidth >= stream_cost) {
            data_->AddDistribution(deal_day, client, max_edge,
                                stream_id, stream_cost);
            days_client_bandwidth_[deal_day][client] -= stream_cost;
            leave_bandwidth -= stream_cost;
            assert(days_client_bandwidth_[deal_day][client] >= 0);
            assert(streams[stream_id] == 0);
            assert(leave_bandwidth >= 0);
          } 
        }
      }
      data_->DelAvailableEdgeNode(deal_day, max_edge);
    }
  }
  return;
}

//void LHKPreDistribution::BalancedDistribute() {
//  //前5%的天数
//  int percent_five_day = std::max((int)(1.0 * (data_->GetAllDays()) * 0.05), 0);
//
//  std::vector<std::string> available_edge_node = edge_node_;
//  int n = available_edge_node.size();
//  int l = allday_;
//
//  //客户机连边数
//  std::unordered_map<std::string, int> client_edge_num;
//  for (std::string &client : client_node_) {
//    client_edge_num[client] = data_->GetClientEdgeNum(client);
//  }
//  //获得每日流量需求
//  days_client_bandwidth_ = std::vector<std::unordered_map<std::string, int>>(l);
//  //每日客户节点连接边缘节点已被处理数
//  std::vector<std::unordered_map<std::string, int>> days_client_deal_nums(l);
//  for (int i = 0; i < allday_; i++) {
//    for (std::string &client : client_node_) {
//      auto streams = data_->GetClientDayDemand(i, client);
//      int demand = 0;
//      for(auto it = streams.begin(); it != streams.end(); ++it){
//        demand += it->second;
//      }
//      days_client_bandwidth_[i][client] = demand;
//    }
//  }
//  //每日使用的边缘节点列表
//  std::vector<std::vector<std::string>> used_edge(l,
//                                                  std::vector<std::string>(0));
//
//  //获得每日的95%节点集合，但是没有真正的分配
//  for (int i = 0; i < n; i++) {
//    long long max_perfent_five = 0, max_bandwidth = 0,
//              max_serving_client_count = 0;
//    //每个边缘节点前5%大的天数的流量总和
//    std::unordered_map<std::string, std::vector<long long>> percent_five{};
//
//    //每个边缘节点前5%大的天数
//    std::unordered_map<std::string, std::vector<int>> percent_days{};
//
//    for (int j = 0; j < n; j++) {
//      percent_five[available_edge_node[j]] = std::vector<long long>(3, 0);
//      percent_five[available_edge_node[j]][1] =
//          data_->GetEdgeNode(available_edge_node[j])->GetBandwidth();
//      percent_five[available_edge_node[j]][2] =
//          data_->GetEdgeNode(available_edge_node[j])->GetservingclientnodeNum();
//      max_bandwidth =
//          std::max(percent_five[available_edge_node[j]][1], max_bandwidth);
//      max_serving_client_count = std::max(
//          percent_five[available_edge_node[j]][2], max_serving_client_count);
//      percent_days[available_edge_node[j]] = std::vector<int>(0);
//    }
//    //统计当前剩下的n-i个边缘节点的每日带宽
//    for (int j = 0; j < n - i; j++) {
//      std::string edge = available_edge_node[j];
//
//      std::vector<long long> edge_bandwidth(l, 0);
//
//      std::vector<int> seq(l);
//
//      for (int k = 0; k < l; k++) seq[k] = k;
//
//      std::unordered_set<std::string> edge_client = data_->GetEdgeClient(edge);
//      for (int k = 0; k < l; k++) {
//        for (auto &client : edge_client) {
//          edge_bandwidth[k] += days_client_bandwidth_[k][client];
//          assert(days_client_bandwidth_[k][client] >= 0);
//          assert(edge_bandwidth[k] >= 0);
//        }
//      }
//
//      std::sort(seq.begin(), seq.end(), [&](const int &a, const int &b) {
//        return edge_bandwidth[a] > edge_bandwidth[b];
//      });
//
//      long long total = 0;
//      for (int k = 0; k < percent_five_day; k++) {
//        total += edge_bandwidth[seq[k]];
//        percent_days[edge].emplace_back(seq[k]);
//      }
//      percent_five[edge][0] = total;
//      max_perfent_five = std::max(total, max_perfent_five);
//    }
//    // 310w: 0.01, 1.0, 0.15
//    // 300w: 0.15, 1.0, 0.1
//    // 300w: 0.05, 1.0, 0.1
//    // 304w: 0.02, 1.0, 0.1
//    // 298w: 0.01, 1.0, 0.1 (minus)
//    // loading : bandwidth : servingcount
//    double w1 = 0.01, w2 = 1.0, w3 = 0.1;
//    std::unordered_map<std::string, double> f;
//    for (std::string edge : available_edge_node) {
//      f[edge] = w1 * percent_five[edge][0] / max_perfent_five +
//                w2 * percent_five[edge][1] / max_bandwidth -
//                w3 * percent_five[edge][2] / max_serving_client_count;
//    }
//    //对边缘节点按照带宽最大5%排序
//    sort(available_edge_node.begin(), available_edge_node.end(),
//         [&](const std::string &a, const std::string &b) {
//           return f[a] < f[b];
//         });
//    //取最大的边缘节点
//    std::string max_edge = available_edge_node[n - i - 1];
//    available_edge_node.resize(n - i - 1);
//
//    std::unordered_set<std::string> edge_client =
//        data_->GetEdgeClient(max_edge);
//    //对每天做处理
//    for (int deal_day : percent_days[max_edge]) {
//      used_edge[deal_day].emplace_back(max_edge);
//      data_->DelAvailableEdgeNode(deal_day, max_edge);
//      //剩余流量
//      int leave_bandwidth = data_->GetEdgeBandwidthLimit(max_edge);
//      std::vector<std::string> client_lists;
//      for (std::string client : edge_client) {
//        client_lists.emplace_back(client);
//      }
//      std::sort(client_lists.begin(), client_lists.end(),
//                [&](const std::string &a, const std::string &b) {
//                  return days_client_bandwidth_[deal_day][a] <
//                         days_client_bandwidth_[deal_day][b];
//                });
//      //若连接数为0，跳过
//      if (client_lists.size() == 0) {
//        continue;
//      }
//      //获得平均分配的流量值
//      int avange_bandwidth =
//          GetAvangeBandwidth(client_lists, leave_bandwidth, deal_day);
//      //进行平均分配
//      for (std::string &client : client_lists) {
//        int occupy = std::min(days_client_bandwidth_[deal_day][client],
//                              avange_bandwidth);
//        days_client_bandwidth_[deal_day][client] -= occupy;
//        leave_bandwidth -= occupy;
//        assert(leave_bandwidth >= 0);
//        days_client_deal_nums[deal_day][client]++;
//      }
//      for (std::string &client : client_lists) {
//        if (leave_bandwidth == 0) break;
//        int occupy = std::min(days_client_bandwidth_[i][client], 1);
//        days_client_bandwidth_[i][client] -= occupy;
//        leave_bandwidth -= occupy;
//        assert(leave_bandwidth >= 0);
//      }
//      leave_bandwidth = 0;
//    }
//  }
//  //进行每日的预分配，对于客户节点连边数较少的实行塞满方式，多的实行均分方式
//  //重新获得每日边缘节点需满足流量
//  for (int i = 0; i < allday_; i++) {
//    for (std::string &client : client_node_) {
//      auto streams = data_->GetClientDayDemand(i, client);
//      int demand = 0;
//      for(auto it = streams.begin(); it != streams.end(); ++it){
//        demand += it->second;
//      }
//      days_client_bandwidth_[i][client] = demand;
//    }
//  }
//  //稀有客户节点的剩余连边数
//  int sub_node_num = 1.0 * edge_node_.size() * RARE_PERCENT;
//
//  //分配
//  for (int i = 0; i < l; i++) {
//    for (std::string &edge : used_edge[i]) {
//      //剩余流量
//      int leave_bandwidth = data_->GetEdgeBandwidthLimit(edge);
//      std::vector<std::string> client_lists, client_pre;
//      //该边缘节点连接的客户节点集合
//      std::unordered_set<std::string> edge_client = data_->GetEdgeClient(edge);
//      for (std::string client : edge_client) {
//        client_pre.emplace_back(client);
//      }
//      //获得一个按照连边数从小到大排序的处理序列
//      std::sort(client_pre.begin(), client_pre.end(),
//                [&](std::string &a, std::string &b) {
//                  return data_->GetClientEdgeNum(a) <
//                         data_->GetClientEdgeNum(b);
//                });
//      //客户节点已经用完所有的边缘节点的及比较危险的节点，先塞满，剩下的放入需平均节点列表中
//      for (std::string client : client_pre) {
//        if (days_client_deal_nums[i][client] >=
//            std::max(client_edge_num[client] - sub_node_num, 0)) {
//          int occupy =
//              std::min(leave_bandwidth, days_client_bandwidth_[i][client]);
//          days_client_bandwidth_[i][client] -= occupy;
//          leave_bandwidth -= occupy;
//          assert(leave_bandwidth >= 0);
//          data_->AddDistribution(i, client, edge, occupy);
//        } else {
//          client_lists.emplace_back(client);
//        }
//      }
//      //若连接数为0，跳过
//      if (client_lists.size() == 0) {
//        continue;
//      }
//      //按照需满足流量从小到大，排序需均衡流量的客户节点
//      std::sort(client_lists.begin(), client_lists.end(),
//                [&](const std::string &a, const std::string &b) {
//                  return days_client_bandwidth_[i][a] <
//                         days_client_bandwidth_[i][b];
//                });
//      //获得平均分配的流量值
//      int avange_bandwidth =
//          GetAvangeBandwidth(client_lists, leave_bandwidth, i);
//      //进行平均分配
//      for (std::string &client : client_lists) {
//        int occupy =
//            std::min(days_client_bandwidth_[i][client], avange_bandwidth);
//        days_client_bandwidth_[i][client] -= occupy;
//        data_->AddDistribution(i, client, edge, occupy);
//        leave_bandwidth -= occupy;
//        assert(leave_bandwidth >= 0);
//      }
//      for (std::string &client : client_lists) {
//        if (leave_bandwidth == 0) break;
//        int occupy = std::min(days_client_bandwidth_[i][client], 1);
//        days_client_bandwidth_[i][client] -= occupy;
//        data_->AddDistribution(i, client, edge, occupy);
//        leave_bandwidth -= occupy;
//        assert(leave_bandwidth >= 0);
//      }
//    }
//  }
//  return;
//}
//void LHKPreDistribution::LHKDistribute() {
//  //前5%的天数
//  int percent_five_day = std::max((int)(1.0 * (data_->GetAllDays()) * 0.05), 0);
//
//  std::vector<std::string> available_edge_node = edge_node_;
//  int n = available_edge_node.size();
//  int l = allday_;
//
//  //客户机连边数
//  std::map<std::string, std::set<std::string>> client_edge_num;
//  for (std::string &client : client_node_) {
//    for (auto &edge : data_->GetClientNode(client)->GetAvailableEdgeNode()) {
//      client_edge_num[client].insert(edge);
//    }
//  }
//
//  //获得每日流量需求
//  days_client_bandwidth_ = std::vector<std::unordered_map<std::string, int>>(l);
//
//  for (int i = 0; i < allday_; i++) {
//    for (std::string &client : client_node_) {
//      auto streams = data_->GetClientDayDemand(i, client);
//      int demand = 0;
//      for(auto it = streams.begin(); it != streams.end(); ++it){
//        demand += it->second;
//      }
//      days_client_bandwidth_[i][client] = demand;
//    }
//  }
//
//  for (int i = 0; i < n; i++) {
//    //每个边缘节点前5%大的天数的流量总和
//    std::unordered_map<std::string, double> percent_five{};
//
//    //每个边缘节点前5%大的天数
//    std::unordered_map<std::string, std::vector<int>> percent_days{};
//
//    for (int j = 0; j < n; j++) {
//      percent_five[available_edge_node[j]] = 0;
//      percent_days[available_edge_node[j]] = std::vector<int>(0);
//    }
//    //统计当前剩下的n-i个边缘节点的每日带宽
//    for (int j = 0; j < n - i; j++) {
//      std::string edge = available_edge_node[j];
//
//      std::vector<double> edge_bandwidth(l, 0);
//      std::vector<int> seq(l);
//
//      for (int k = 0; k < l; k++) seq[k] = k;
//
//      std::unordered_set<std::string> edge_client = data_->GetEdgeClient(edge);
//
//      std::set<std::string> link_counter;
//      for (auto &client : edge_client) {
//        for (auto &nei_edge_name : client_edge_num[client]) {
//          link_counter.insert(nei_edge_name);
//        }
//      }
//      for (int k = 0; k < l; k++) {
//        for (auto &client : edge_client) {
//          edge_bandwidth[k] += days_client_bandwidth_[k][client];
//          assert(days_client_bandwidth_[k][client] >= 0);
//          // assert(edge_bandwidth[k] >= 0);
//        }
//        edge_bandwidth[k] = std::fabs(
//            (data_->GetEdgeNode(edge)->GetBandwidth() - edge_bandwidth[k]) /
//            (1 + link_counter.size()));
//      }
//
//      std::sort(seq.begin(), seq.end(), [&](const int &a, const int &b) {
//        return edge_bandwidth[a] > edge_bandwidth[b];
//      });
//
//      double total = 0.0f;
//      for (int k = 0; k < percent_five_day; k++) {
//        total += edge_bandwidth[seq[k]];
//        percent_days[edge].emplace_back(seq[k]);
//      }
//      percent_five[edge] = total;
//    }
//    //对边缘节点按照带宽最大5%排序
//    sort(available_edge_node.begin(), available_edge_node.end(),
//         [&](const std::string &a, const std::string &b) {
//           return percent_five[a] > percent_five[b];
//         });
//
//    //取最大的边缘节点
//    std::string max_edge = available_edge_node[available_edge_node.size() - 1];
//    available_edge_node.resize(n - i - 1);
//
//    std::unordered_set<std::string> edge_client =
//        data_->GetEdgeClient(max_edge);
//    std::vector<std::string> client_lists;
//    //按照客户端连边数对客户节点排序
//    for (std::string client : edge_client) client_lists.emplace_back(client);
//    sort(client_lists.begin(), client_lists.end(),
//         [&](const std::string &a, const std::string &b) {
//           return client_edge_num[a] < client_edge_num[b];
//         });
//    //对每天做处理
//    for (int deal_day : percent_days[max_edge]) {
//      //剩余流量
//      int leave_bandwidth = data_->GetEdgeBandwidthLimit(max_edge);
//
//      assert(leave_bandwidth >= 0);
//
//      //按照客户端节点连边数从小到大更新
//      for (std::string client : client_lists) {
//        if (leave_bandwidth == 0) break;
//        if (days_client_bandwidth_[deal_day][client] == 0) continue;
//
//        if (leave_bandwidth >= days_client_bandwidth_[deal_day][client]) {
//          leave_bandwidth -= days_client_bandwidth_[deal_day][client];
//          data_->AddDistribution(deal_day, client, max_edge,
//                                 days_client_bandwidth_[deal_day][client]);
//          days_client_bandwidth_[deal_day][client] = 0;
//          assert(leave_bandwidth >= 0);
//
//        } else {
//          data_->AddDistribution(deal_day, client, max_edge, leave_bandwidth);
//          days_client_bandwidth_[deal_day][client] -= leave_bandwidth;
//          assert(days_client_bandwidth_[deal_day][client] >= 0);
//          leave_bandwidth = 0;
//        }
//      }
//      data_->DelAvailableEdgeNode(deal_day, max_edge);
//    }
//  }
//  return;
//}

void LHKPreDistribution::GetEdgeOrder() {
  std::vector<long long> bandwidth;  //用以排序并获取大流量值的流量和数组
  bandwidth.reserve(allday_ * edge_node_.size());

  //每日边缘节点的需要满足的带宽最大量
  std::vector<std::unordered_map<std::string, long long>> days_edge_bandwidth(
      allday_, std::unordered_map<std::string, long long>{});

  //最终获得的边缘节点顺序
  std::vector<std::string> edge_order;

  //获得每日边缘节点的需要满足的带宽最大量
  for (std::string edge : edge_node_) {
    edge_order.emplace_back(edge);
    std::unordered_set<std::string> edge_client = data_->GetEdgeClient(edge);
    for (int i = 0; i < allday_; i++) {
      long long tmp_bandwidth = 0;
      for (std::string client : edge_client) {
        tmp_bandwidth += days_client_bandwidth_[i][client];
      }
      days_edge_bandwidth[i][edge] = tmp_bandwidth;
      bandwidth.emplace_back(tmp_bandwidth);
    }
  }
  //排序并取出大流量最低的流量要求值
  sort(bandwidth.begin(), bandwidth.end());

  long long up_value = bandwidth[std::max(
      (int)(bandwidth.size() * UP_EDGE_BANDWIDTH_PERCENT - 1), 0)];

  //对每个边缘节点的大流量值天数计数

  std::unordered_map<std::string, int> edge_big_bandwidth;
  for (int i = 0; i < allday_; i++) {
    for (std::string edge : edge_node_) {
      if (days_edge_bandwidth[i][edge] >= up_value) {
        edge_big_bandwidth[edge]++;
      }
    }
  }
  //按照连接节点数为第一优先级从大到小，流量大小为第二优先级从大到小排序
  sort(edge_order.begin(), edge_order.end(),
       [&](std::string &a, std::string &b) {
         if (data_->GetEdgeClientNum(a) == data_->GetEdgeClientNum(b))
           return edge_big_bandwidth[a] > edge_big_bandwidth[b];
         else
           return data_->GetEdgeClientNum(a) > data_->GetEdgeClientNum(b);
       });
  int n = edge_order.size();
  for (int i = 0; i < n; i++) data_->SetEdgeNodeOrder(edge_order[i], i);
}

void LHKPreDistribution::GetClientOrder() {
  std::vector<long long> bandwidth;  //用以排序并获取大流量值的流量和数组
  bandwidth.reserve(allday_ * client_node_.size());

  //最终获得的客户节点顺序
  std::vector<std::string> client_order;

  //排序并取出大流量最低的流量要求值
  sort(bandwidth.begin(), bandwidth.end());

  long long up_value = bandwidth[std::max(
      (int)(bandwidth.size() * UP_CLIENT_BANDWIDTH_PERCENT - 1), 0)];

  //对每个客户节点的大流量值天数计数

  std::unordered_map<std::string, int> client_big_bandwidth;
  for (int i = 0; i < allday_; i++) {
    for (std::string client : client_node_) {
      if (days_client_bandwidth_[i][client] >= up_value) {
        client_big_bandwidth[client]++;
      }
    }
  }

  //按照连接节点数为第一优先级，流量大小为第二优先级从小到大排序
  sort(client_order.begin(), client_order.end(),
       [&](std::string &a, std::string &b) {
         if (data_->GetClientEdgeNum(a) == data_->GetClientEdgeNum(b))
           return client_big_bandwidth[a] < client_big_bandwidth[b];
         else
           return data_->GetClientEdgeNum(a) < data_->GetClientEdgeNum(b);
       });
  int n = client_order.size();
  for (int i = 0; i < n; i++) data_->SetClientNodeOrder(client_order[i], i);
}

void LHKPreDistribution::GetDaysOrder() {
  std::vector<int> client_bandwidth;
  //获取每日流量和
  std::vector<long long> days_bandwidth(allday_, 0);
  for (int i = 0; i < allday_; i++) {
    for (auto &p : days_client_bandwidth_[i]) {
      std::string client = p.first;
      int bandwidth = p.second;
      days_bandwidth[i] += bandwidth;
      client_bandwidth.emplace_back(bandwidth);
    }
  }
  //获取大流量下限
  int up_value =
      client_bandwidth[std::max((int)(client_bandwidth.size() * 0.5 - 1), 0)];
  //统计每日大流量需求数
  std::vector<int> days_big(allday_, 0);
  for (int i = 0; i < allday_; i++) {
    for (auto &p : days_client_bandwidth_[i]) {
      std::string client = p.first;
      int bandwidth = p.second;
      if (bandwidth >= up_value) days_big[i]++;
    }
  }
  std::vector<int> days_order(allday_);
  //先按大流量需求数排序，若需求相同按流量和排序
  for (int i = 0; i < allday_; i++) days_order[i] = i;
  std::sort(days_order.begin(), days_order.end(),
            [&](const int &a, const int &b) {
              return (days_big[a] == days_big[b])
                         ? (days_bandwidth[a] < days_bandwidth[b])
                         : (days_big[a] < days_big[b]);
            });
  data_->SetDaysOrder(days_order);
  return;
}