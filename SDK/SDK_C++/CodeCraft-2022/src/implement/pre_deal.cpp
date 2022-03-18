
#include "pre_deal.h"

void PreDistribution::Distribute() {
  int percent_five_day = fmax(1.0 * alldays * 0.05 - 1,0);

  std::vector<std::string> available_edge_node;
  std::unordered_set<std::string> chose_nodes;

  distribution_.clear();
  distribution_.resize(alldays);

  for (auto p : edge_node_) {
    available_edge_node.emplace_back(p.first);
    chose_nodes.insert(p.first);
  }

  int n = available_edge_node.size();
  int m = client_node_.size();
  int l = alldays;

  //获得每天带宽需求

  std::vector<std::unordered_map<std::string, int>> days_client_bandwidth(
      l, std::unordered_map<std::string, int>{});

  for (auto p : client_node_) {
    std::string client = p.first;
    ClientNode *client_msg = p.second;
    assert(client_msg != nullptr);
    for (int i = 0; i < l; i++) {
      days_client_bandwidth[i][client] = client_msg->GetDemand(i);
    }
  }

  //获得连边情况
  std::unordered_map<std::string, std::unordered_set<std::string>> edge_client;

  //客户机连边数
  std::unordered_map<std::string, int> client_edge_num;

  for (auto &p : edge_node_) {
    std::string edge = p.first;
    EdgeNode *edge_msg = p.second;
    edge_client[edge] = edge_msg->Getservingclientnode();
    for (auto &client : edge_client[edge]) client_edge_num[client]++;
  }

  for (int i = 0; i < l; i++) {
    available_edge_node_[i] = chose_nodes;
  }

  for (int i = 0; i < n; i++) {
    //每个边缘节点前5%大的天数的流量总和
    std::unordered_map<std::string, long long> percent_five{};

    //每个边缘节点前5%大的天数

    std::unordered_map<std::string, std::vector<int>> percent_days{};

    for (int j = 0; j < n; j++) {
      percent_five[available_edge_node[j]] = 0;
      percent_days[available_edge_node[j]] = std::vector<int>(0);
    }

    //统计当前剩下的n-i个边缘节点的每日带宽
    for (int j = 0; j < n - i; j++) {

      std::string edge = available_edge_node[j];

      std::vector<long long> edge_bandwidth(l, 0);
      std::vector<int> seq(l);

      for (int k = 0; k < l; k++) seq[k] = k;
      for (int k = 0; k < l; k++) {
        for (auto &client : edge_client[edge]) {
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
      percent_five[edge] = total;
    }

    //对边缘节点按照带宽最大5%排序
    sort(available_edge_node.begin(), available_edge_node.end(),
         [&](const std::string &a, const std::string &b) {
           return percent_five[a] < percent_five[b];
         });

    //取最大的边缘节点
    std::string max_edge = available_edge_node[available_edge_node.size() - 1];
    available_edge_node.resize(n - i - 1);

    //对每天做处理
    for (int deal_day : percent_days[max_edge]) {

      //按照客户端连边数对客户节点排序
      std::vector<std::string> client_lists;
      for (std::string client : edge_client[max_edge])
        if(days_client_bandwidth[deal_day][client] > 0) client_lists.emplace_back(client);
      sort(client_lists.begin(), client_lists.end(),
           [&](const std::string &a, const std::string &b) {
             return client_edge_num[a] < client_edge_num[b];
           });
      //剩余流量
      int leave_bandwidth = edge_node_[max_edge]->GetBandwidth();
      assert(leave_bandwidth >= 0);

      //按照客户端节点连边数从小到大更新
      for (std::string client : client_lists) {
        if (leave_bandwidth == 0) break;
        if (days_client_bandwidth[deal_day][client] == 0) continue;

        if (leave_bandwidth >= days_client_bandwidth[deal_day][client]) {
          leave_bandwidth -= days_client_bandwidth[deal_day][client];
          distribution_[deal_day][client][max_edge] +=
              days_client_bandwidth[deal_day][client];
          days_client_bandwidth[deal_day][client] = 0;
          assert(leave_bandwidth >= 0);

        } else {

          distribution_[deal_day][client][max_edge] += leave_bandwidth;
          days_client_bandwidth[deal_day][client] -= leave_bandwidth;
          assert(days_client_bandwidth[deal_day][client] >= 0);
          leave_bandwidth = 0;
        }
      }
      available_edge_node_[deal_day].erase(max_edge);
    }
  }
  return;
}
