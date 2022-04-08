
#include "lhl_pre_deal.h"

void LHLPreDistribution::Distribute() {
  LHLDistribute();

  // update data
  GetDaysOrder();
  return ;
}

void LHLPreDistribution::LHLDistribute() {
  //前5%的天数
  int percent_five_day = std::max((int)(1.0 * (data_->GetAllDays()) * 0.04), 0);

  std::vector<std::string> available_edge_node = edge_node_;
  int n = available_edge_node.size();
  int l = allday_;

  //客户机连边数
  std::unordered_map<std::string, int> client_edge_num;
  for (std::string &client : client_node_) {
    client_edge_num[client] = data_->GetClientEdgeNum(client);
  }
  //获得每日流量总需求
  days_client_bandwidth_ = std::vector<std::unordered_map<std::string, int>>(l);

  for (int i = 0; i < allday_; i++) {
    for (std::string &client : client_node_) {
      std::unordered_map<std::string,int> streams = data_->GetClientDayRemainingDemand(i, client);
      int demand = 0;
      for(auto it = streams.begin(); it != streams.end(); ++it){
        demand += it->second;
      }
      days_client_bandwidth_[i][client] = demand;
      assert(demand >= 0);
    }
  }

  //获得每日的95%节点集合,并分配
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
    double w1 = 0.03, w2 = 1.0, w3 = 0.05;
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
        
        std::unordered_map<std::string,int> streams = data_->GetClientDayRemainingDemand(deal_day, client);
        std::vector<std::string> ordered_stream_id;
        for(auto &p : streams){
          ordered_stream_id.emplace_back(p.first);
        }

        std::sort(ordered_stream_id.begin(), ordered_stream_id.end(), [&](std::string &a, std::string &b){
          return streams[a] > streams[b];
        });

        for(auto &stream_id : ordered_stream_id){
          int &stream_cost = streams[stream_id];

          if(stream_cost == 0) continue;

          if (leave_bandwidth >= stream_cost) {
            data_->SetDistribution(deal_day,client,stream_id,max_edge);
            days_client_bandwidth_[deal_day][client] -= stream_cost;
            leave_bandwidth -= stream_cost;
            assert(days_client_bandwidth_[deal_day][client] >= 0);
            assert(leave_bandwidth >= 0);
          } 
        }
      }
      data_->DelAvailableEdgeNode(deal_day, max_edge);
    }
  }
  return ;
}


void LHLPreDistribution::GetDaysOrder() {
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
  return ;
}