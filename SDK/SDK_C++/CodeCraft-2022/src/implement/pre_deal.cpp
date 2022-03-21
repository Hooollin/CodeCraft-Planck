
#include "pre_deal.h"

void PreDistribution::Distribute() {
  //前5%的天数
  int percent_five_day =
      std::max((int)(1.0 * (data_->GetAllDays()) * 0.05), 0);

  std::vector<std::string> available_edge_node = edge_node_;
  int n = available_edge_node.size();
  int l = allday_;

  //客户机连边数
  std::unordered_map<std::string, int> client_edge_num;
  for (std::string &client : client_node_) {
    client_edge_num[client] = data_->GetClientEdgeNum(client);
  }
  //获得每日流量需求
  std::vector<std::unordered_map<std::string, int>> days_client_bandwidth(l);

  for (int i = 0; i < allday_; i++) {
    for (std::string &client : client_node_) {
      days_client_bandwidth[i][client] = data_->GetClientDayDemand(i, client);
    }
  }

  for (int i = 0; i < n; i++) {
    long long max_perfent_five = 0, max_bandwidth = 0, max_serving_client_count = 0;
    //每个边缘节点前5%大的天数的流量总和
    std::unordered_map<std::string, std::vector<long long>> percent_five{};

    //每个边缘节点前5%大的天数
    std::unordered_map<std::string, std::vector<int>> percent_days{};

    for (int j = 0; j < n; j++) {
      percent_five[available_edge_node[j]] = std::vector<long long>(3, 0);
      percent_five[available_edge_node[j]][1] = data_->GetEdgeNode(available_edge_node[j])->GetBandwidth();
      percent_five[available_edge_node[j]][2] = data_->GetEdgeNode(available_edge_node[j])->GetservingclientnodeNum();
      max_bandwidth = std::max(percent_five[available_edge_node[j]][1], max_bandwidth);
      max_serving_client_count = std::max(percent_five[available_edge_node[j]][2], max_serving_client_count);

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
          edge_bandwidth[k] += days_client_bandwidth[k][client];
          assert(days_client_bandwidth[k][client] >= 0);
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
    //对边缘节点按照带宽最大5%排序
    sort(available_edge_node.begin(), available_edge_node.end(),
         [&](const std::string &a, const std::string &b) {
           // 45w: 0.0, 1.0, 0.5
           // 43.6w: 0.0, 2.0, 0.5
           // loading : bandwidth : servingcount
           double w1 = 0.1, w2 = 0.0, w3 = 1.0;
           double f1 = w1 * percent_five[a][0] / max_perfent_five + w2 * percent_five[a][1] / max_bandwidth + w3 * percent_five[a][2] / max_serving_client_count;
           double f2 = w1 * percent_five[b][0] / max_perfent_five + w2 * percent_five[b][1] / max_bandwidth + w3 * percent_five[b][2] / max_serving_client_count;
           return f1 < f2;
         });

    //取最大的边缘节点
    std::string max_edge = available_edge_node[available_edge_node.size() - 1];
    available_edge_node.resize(n - i - 1);

    std::unordered_set<std::string> edge_client =
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
      for (std::string client : client_lists) {
        if (leave_bandwidth == 0) break;
        if (days_client_bandwidth[deal_day][client] == 0) continue;

        if (leave_bandwidth >= days_client_bandwidth[deal_day][client]) {
          leave_bandwidth -= days_client_bandwidth[deal_day][client];
          data_->AddDistribution(deal_day, client, max_edge,
                                 days_client_bandwidth[deal_day][client]);
          days_client_bandwidth[deal_day][client] = 0;
          assert(leave_bandwidth >= 0);

        } else {
          data_->AddDistribution(deal_day, client, max_edge, leave_bandwidth);
          days_client_bandwidth[deal_day][client] -= leave_bandwidth;
          assert(days_client_bandwidth[deal_day][client] >= 0);
          leave_bandwidth = 0;
        }
      }
      data_->DelAvailableEdgeNode(deal_day,max_edge);
    }
  }
  return;
}

void PreDistribution::GetEdgeOrder() {
  std::vector<long long> bandwidth;  //用以排序并获取大流量值的流量和数组

  bandwidth.reserve(allday_ * edge_node_.size());

  //获得每日客户节点的带宽需求量
  std::vector<std::unordered_map<std::string, int>> days_client_bandwidth;
  for (int i = 0; i < allday_; i++) {
    for (std::string &client : client_node_) {
      days_client_bandwidth[i][client] = data_->GetClientDayDemand(i, client);
    }
  }

  //每日边缘节点的需要满足的带宽最大量
  std::vector<std::unordered_map<std::string, long long>> days_edge_bandwidth(
      allday_, std::unordered_map<std::string, long long>{});

  //最终获得的边缘节点顺序
  std::vector<std::string> edge_order;

  //获得每日边缘节点的需要满足的带宽最大量
  for (std::string edge : edge_node_) {
    edge_order.emplace_back(edge);
    for (int i = 0; i < allday_; i++) {
      long long tmp_bandwidth = 0;
      std::unordered_set<std::string> edge_client = data_->GetEdgeClient(edge);
      for (std::string client : edge_client) {
        tmp_bandwidth += days_client_bandwidth[i][client];
      }
      days_edge_bandwidth[i][edge] = tmp_bandwidth;
      bandwidth.emplace_back(tmp_bandwidth);
    }
  }

  //排序并取出大流量最低的流量要求值
  sort(bandwidth.begin(), bandwidth.end());

  long long up_value = bandwidth[std::max(
      (int)(bandwidth.size() * UP_BANDWIDTH_PERCENT - 1), 0)];

  //对每个边缘节点的大流量值天数计数

  std::unordered_map<std::string, int> edge_big_bandwidth;
  for (int i = 0; i < allday_; i++) {
    for (std::string edge : edge_order) {
      for (int j = 0; j < allday_; j++) {
        if (days_edge_bandwidth[j][edge] >= up_value)
          edge_big_bandwidth[edge]++;
      }
    }
  }

  //按照连接节点数为第一优先级，流量大小为第二优先级从大到小排序
  sort(edge_order.begin(), edge_order.end(),
       [&](std::string &a, std::string &b) {
         if (data_->GetEdgeClientNum(a) == data_->GetEdgeClientNum(b))
           return edge_big_bandwidth[a] > edge_big_bandwidth[b];
         else
           return data_->GetEdgeClientNum(a) < data_->GetEdgeClientNum(b);
       });

  data_->SetEdgeNodeOrder(edge_order);
}

void PreDistribution::GetClientOrder(){

}