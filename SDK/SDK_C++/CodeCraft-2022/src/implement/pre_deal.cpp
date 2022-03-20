
#include "pre_deal.h"

PreDistribution::PreDistribution(
    std::unordered_map<std::string, EdgeNode *> edge_node,
    std::unordered_map<std::string, ClientNode *> client_node)
    : edge_node_(edge_node),
      client_node_(client_node),
      alldays(client_node.begin()->second->GetDays()) {
  available_edge_node_.clear();
  available_edge_node_.resize(alldays);
  distribution_.clear();
  distribution_.resize(alldays);

  //获得每天带宽需求
  days_client_bandwidth_ = std::vector<std::unordered_map<std::string, int>>(
      alldays, std::unordered_map<std::string, int>{});
  for (auto &p : client_node) {
    std::string client = p.first;
    ClientNode *client_msg = p.second;
    assert(client_msg != nullptr);
    for (int i = 0; i < alldays; i++) {
      days_client_bandwidth_[i][client] = client_msg->GetDemand(i);
    }
  }
  //获得连边情况
  for (auto &p : edge_node) {
    std::string edge = p.first;
    EdgeNode *edge_msg = p.second;
    edge_client_[edge] = edge_msg->Getservingclientnode();
  }

  for (auto &p : client_node) {
    std::string client = p.first;
    ClientNode *client_msg = p.second;
    client_edge_[client] = client_msg->GetAvailableEdgeNode();
  }
}

void PreDistribution::Distribute() {
  int percent_five_day = std::max((int)(1.0 * alldays * 0.05 - 1), 0);

  std::vector<std::string> available_edge_node;
  std::unordered_set<std::string> chose_nodes;

  distribution_.clear();
  distribution_.resize(alldays);

  for (auto p : edge_node_) {
    available_edge_node.emplace_back(p.first);
    chose_nodes.insert(p.first);
  }

  int n = available_edge_node.size();
  int l = alldays;

  std::vector<std::unordered_map<std::string, int>> days_client_bandwidth =
      days_client_bandwidth_;

  //客户机连边数
  std::unordered_map<std::string, int> client_edge_num;

  for (auto &p : client_edge_) {
    std::string client = p.first;
    client_edge_num[client] = p.second.size();
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
        for (auto &client : edge_client_[edge]) {
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
      for (std::string client : edge_client_[max_edge])
        if (days_client_bandwidth[deal_day][client] > 0)
          client_lists.emplace_back(client);
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

std::vector<std::string> PreDistribution::GetEdgeOrder() {
  std::vector<long long> bandwidth;  //用以排序并获取大流量值的流量和数组

  bandwidth.reserve(alldays * edge_node_.size());

  //每日客户节点的带宽需求量
  std::vector<std::unordered_map<std::string, int> > days_client_bandwidth =
      days_client_bandwidth_;

  //每日边缘节点的需要满足的带宽最大量
  std::vector<std::unordered_map<std::string, long long>> days_edge_bandwidth(
      alldays, std::unordered_map<std::string, long long>{});

  //最终获得的边缘节点顺序
  std::vector<std::string> edge_order;

  //获得每日边缘节点的需要满足的带宽最大量
  for (auto &p : edge_node_) {
    std::string edge = p.first;
    edge_order.emplace_back(edge);
    for (int i = 0; i < alldays; i++) {
      long long tmp_bandwidth = 0;
      for (std::string client : edge_client_[edge]) {
        tmp_bandwidth += days_client_bandwidth[i][client];
      }
      days_edge_bandwidth[i][edge] = tmp_bandwidth;
      bandwidth.emplace_back(tmp_bandwidth);
    }
  }

  //排序并取出大流量最低的流量要求值
  sort(bandwidth.begin(),bandwidth.end());

  long long up_value = bandwidth[std::max((int)(bandwidth.size() * UP_BANDWIDTH_PERCENT - 1),0)];

  //对每个边缘节点的大流量值天数计数

  std::unordered_map<std::string,int> edge_big_bandwidth;
  for(int i=0;i<alldays;i++){
    for(std::string edge : edge_order){
      for(int j=0;j<alldays;j++){
        if(days_edge_bandwidth[j][edge] >= up_value) edge_big_bandwidth[edge] ++;
      }
    }
  }

  //按照连接节点数为第一优先级，流量大小为第二优先级从大到小排序
  sort(edge_order.begin(),edge_order.end(),[&](const std::string &a, const std::string &b){
    if(edge_client_[a].size() == edge_client_[b].size()) return edge_big_bandwidth[a] > edge_big_bandwidth[b];
    else return edge_client_[a].size() > edge_client_[b].size();
  });

  return edge_order;
}
