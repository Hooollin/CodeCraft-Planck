
#include "distrubution_strategy.h"

DayDistribution::DayDistribution(int &day, Data *data) {
  data_ = data;
  edge_client_node_.clear();
  client_edge_node_.clear();
  client_bandwidth_.clear();
  edge_bandwidth_.clear();
  //只输入可以使用的边缘节点信息，并利用信息赋值
  edge_node_ = data_->GetAvailableEdgeNode(day);

  for (std::string edge : edge_node_) {
    edge_client_node_[edge] = data_->GetEdgeClient(edge);
    edge_bandwidth_[edge] = 0;
  }

  //赋值客户节点信息，并且关联的客户节点为可以使用的边缘节点
  client_node_ = data_->GetClientSet();

  for (std::string client : client_node_) {
    std::unordered_set<std::string> tmp = data_->GetClientEdge(client);
    for (auto &pp : tmp) {
      if (edge_node_.find(pp) != edge_node_.end())
        client_edge_node_[client].insert(pp);
    }
    //更新客户节点剩余需满足的带宽大小
    std::unordered_map<std::string, int> distribution =
        data_->GetDistribution(day, client);
    for (auto &pp : distribution) {
      std::string edge_ = pp.first;
      int value = pp.second;
      client_bandwidth_[client] -= value;
    }
    client_bandwidth_[client] += data_->GetClientDayDemand(day, client);
  }
  days_ = day;
}
void ClientDayDistribution::DistributeBalanced() {
  std::vector<std::string> client_order_;
  for (std::string client : client_node_) {
    client_order_.emplace_back(client);
  }
  sort(client_order_.begin(), client_order_.end(),
       [&](const std::string &a, const std::string &b) {
         return client_edge_node_[a].size() < client_edge_node_[b].size();
       });
  for (std::string client : client_order_) {
    if (client_bandwidth_[client] == 0) continue;
    //获得一个该客户节点连接的边缘节点序列，按照边缘节点当前流量量从小到大排序
    std::vector<std::string> connect_edge;
    int client_bandwidth = client_bandwidth_[client];
    for (auto &p : client_edge_node_[client]) {
      connect_edge.emplace_back(p);
    }
    int n = connect_edge.size();
    sort(connect_edge.begin(), connect_edge.end(),
         [&](const std::string a, const std::string b) {
           return edge_bandwidth_[a] < edge_bandwidth_[b];
         });
    //找寻一个m下标，可以使得流量分配后第1~m个边缘节点的流量分配情况尽可能相等，但流量不超过第m+1个边缘节点
    int max_bandwidth = edge_bandwidth_[connect_edge[n - 1]];
    assert(max_bandwidth >= 0);
    int all_dif = 0;
    for (int i = 0; i < n; i++) {
      all_dif += max_bandwidth - edge_bandwidth_[connect_edge[i]];
      assert(all_dif >= 0);
    }
    int m = n;
    for (int i = n - 2; i >= 0; i--) {
      if (all_dif <= client_bandwidth) break;
      int dif = (edge_bandwidth_[connect_edge[i + 1]] -
                 edge_bandwidth_[connect_edge[i]]) *
                (m - 1);
      all_dif -= dif;
      assert(all_dif >= 0);
      max_bandwidth = edge_bandwidth_[connect_edge[i]];
      m--;
    }
    int leave_bandwidth = 0;
    //分配客户机流量，使得1~m这几个边缘节点的流量使用情况尽量相等，并对超出上限流量进行减法去掉
    int all_bandwidth = max_bandwidth + (client_bandwidth - all_dif) / m;
    int less_num = m - (client_bandwidth - all_dif) % m;
    for (int i = 0; i < less_num; i++) {
      std::string edge = connect_edge[i];
      int edge_bandwidth_limit = data_->GetEdgeBandwidthLimit(edge);
      if (all_bandwidth <= edge_bandwidth_limit) {
        data_->AddDistribution(days_, client, edge,
                               all_bandwidth - edge_bandwidth_[edge]);
        edge_bandwidth_[edge] = all_bandwidth;
      } else {
        data_->AddDistribution(days_, client, edge,
                               edge_bandwidth_limit - edge_bandwidth_[edge]);
        edge_bandwidth_[edge] = edge_bandwidth_limit;
        leave_bandwidth += all_bandwidth - edge_bandwidth_limit;
      }
    }
    for (int i = less_num; i < m; i++) {
      std::string edge = connect_edge[i];
      int edge_bandwidth_limit = data_->GetEdgeBandwidthLimit(edge);
      if (all_bandwidth + 1 <= edge_bandwidth_limit) {
        data_->AddDistribution(days_, client, edge,
                               all_bandwidth + 1 - edge_bandwidth_[edge]);
        edge_bandwidth_[edge] = all_bandwidth + 1;
      } else {
        data_->AddDistribution(days_, client, edge,
                              edge_bandwidth_limit - edge_bandwidth_[edge]);
        edge_bandwidth_[edge] = edge_bandwidth_limit;
        leave_bandwidth += all_bandwidth - edge_bandwidth_limit + 1;
      }
    }
    sort(connect_edge.begin(), connect_edge.end(),
         [&](std::string &a,  std::string &b) {
           return data_->GetEdgeBandwidthLimit(a) - edge_bandwidth_[a] >
                  data_->GetEdgeBandwidthLimit(b) - edge_bandwidth_[b];
         });
    for (int i = 0; i < n; i++) {
      std::string edge = connect_edge[i];
      if (leave_bandwidth == 0) break;
      int edge_bandwidth_limit = data_->GetEdgeBandwidthLimit(edge);
      if (edge_bandwidth_limit - edge_bandwidth_[edge] >= leave_bandwidth) {
        edge_bandwidth_[edge] += leave_bandwidth;
        data_->AddDistribution(days_,client,edge,leave_bandwidth);
        leave_bandwidth = 0;
        break;
      } else {
        data_->AddDistribution(days_,client,edge,edge_bandwidth_limit - edge_bandwidth_[edge]);
        leave_bandwidth -= edge_bandwidth_limit - edge_bandwidth_[edge];
        edge_bandwidth_[edge] = edge_bandwidth_limit;
      }
    }
    client_bandwidth_[client] = 0;
  }
}

void ClientDayDistribution::DistributeForMyBest() {
  std::vector<std::string> client_order_;
  for (std::string client : client_node_) {
    client_order_.emplace_back(client);
  }
  sort(client_order_.begin(), client_order_.end(),
       [&](const std::string &a, const std::string &b) {
         return client_edge_node_[a].size() < client_edge_node_[b].size();
       });
  for (std::string client : client_order_) {
    if (client_bandwidth_[client] == 0) continue;
    //获得一个该客户节点连接的边缘节点序列，按照边缘节点当前流量量从小到大排序
    std::vector<std::string> connect_edge;
    int client_bandwidth = client_bandwidth_[client];
    for (auto &p : client_edge_node_[client]) {
      connect_edge.emplace_back(p);
    }
    int n = connect_edge.size();
    sort(connect_edge.begin(), connect_edge.end(),
         [&](const std::string a, const std::string b) {
           return edge_bandwidth_[a] < edge_bandwidth_[b];
         });

    //尽力而为分配
    int leave_bandwidth = client_bandwidth;
    for (std::string &edge : connect_edge) {
      if (leave_bandwidth == 0) break;
      int edge_bandwidth_limit = data_->GetEdgeBandwidthLimit(edge);
      if (edge_bandwidth_limit - edge_bandwidth_[edge] >= leave_bandwidth) {
        edge_bandwidth_[edge] += leave_bandwidth;
        data_->AddDistribution(days_,client,edge,leave_bandwidth);
        leave_bandwidth = 0;
        break;
      } else {
        data_->AddDistribution(days_,client,edge,edge_bandwidth_limit - edge_bandwidth_[edge]);
        leave_bandwidth -= edge_bandwidth_limit - edge_bandwidth_[edge];
        edge_bandwidth_[edge] = edge_bandwidth_limit;
      }
    }
    leave_bandwidth = 0;
  }
}