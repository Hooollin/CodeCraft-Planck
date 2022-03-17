#include "distrubution_strategy.h"
#include <bits/stdc++.h>

DayDistribution::DayDistribution(
    int day, std::unordered_map<std::string, EdgeNode *> &edge_node,
    std::unordered_map<std::string, ClientNode *> &client_node,
    std::unordered_map<std::string, std::unordered_map<std::string, int> >
        &distribution,
    std::unordered_set<std::string> &avaliable_edge_node) {
  edge_client_node_.clear();
  client_client_node_.clear();
  distribution_ = distribution;
  client_bandwidth_.clear();

  //只输入可以使用的边缘节点信息，并利用信息赋值
  for (auto &p : avaliable_edge_node) {
    edge_client_node_[p] = edge_node[p]->Getservingclientnode();
    edge_up_[p] = edge_node[p]->GetBandwidth();
    edge_bandwidth_[p] = 0;
    edge_cost_[p] = edge_node[p]->GetCost();
  }
  //赋值客户节点信息，并且关联的边缘节点为可以使用的边缘节点
  for (auto &p : client_node) {
    std::string name = p.first;
    ClientNode *client = p.second;
    std::unordered_set<std::string> tmp = client->GetAvailableEdgeNode();
    for (auto &pp : tmp) {
      if (avaliable_edge_node.find(pp) != avaliable_edge_node.end())
        client_client_node_[name].insert(pp);
    }
    //更新客户节点剩余需满足的带宽大小
    for (auto &pp : distribution[name]) {
      std::string edge_name = pp.first;
      int value = pp.second;
      client_bandwidth_[name] -= value;
    }
    client_bandwidth_[name] += client->GetDemand(day);
  }
}

void SimplyDayDistribution::distribute() {
  for (std::string &client : client_order_) {
    //获得一个该客户节点连接的边缘节点序列，按照边缘节点当前流量量从小到大排序
    std::vector<std::string> connect_edge;
    int client_bandwidth = client_bandwidth_[client];
    for (auto &p : client_client_node_[client]) {
      connect_edge.emplace_back(p);
    }
    int n = connect_edge.size();
    sort(connect_edge.begin(), connect_edge.end(),
         [&](const std::string a, const std::string b) {
           return edge_bandwidth_[a] < edge_bandwidth_[b];
         });

    //找寻一个m下标，可以使得流量分配后第1~m个边缘节点的流量分配情况尽可能相等，但流量不超过第m+1个边缘节点
    int max_bandwidth = edge_bandwidth_[connect_edge[n - 1]];
    int all_dif = 0;
    for (int i = 0; i < n - 1; i++)
      all_dif += max_bandwidth - edge_bandwidth_[connect_edge[i]];
    int m = n;
    for (int i = n - 2; i >= 0; i--) {
      if (all_dif <= client_bandwidth) break;
      int dif = edge_bandwidth_[connect_edge[i]] -
                edge_bandwidth_[connect_edge[i + 1]] * (m - 1);
      all_dif -= dif;
      m--;
    }

    //分配客户机流量，使得1~m这几个边缘节点的流量使用情况尽量相等
    int all_bandwidth = max_bandwidth + (client_bandwidth - all_dif) / m;
    int less_num = m - (max_bandwidth + (client_bandwidth - all_dif)) % m;
    for (int i = 0; i < less_num; i++) {
      std::string edge = connect_edge[i];
      distribution_[client][edge] += all_bandwidth - edge_bandwidth_[edge];
      edge_bandwidth_[edge] = all_bandwidth;
    }
    for (int i = less_num; i < m; i++) {
      std::string edge = connect_edge[i];
      distribution_[client][edge] += all_bandwidth + 1 - edge_bandwidth_[edge];
      edge_bandwidth_[edge] = all_bandwidth + 1;
    }

    client_bandwidth_[client] = 0;
  }
}
