
#include "distrubution_strategy.h"

void ClientDayDistribution::Distribute() { DistributeBalanced(); }

DayDistribution::DayDistribution(int &day, Data *data) {
  data_ = data;
  edge_client_node_.clear();
  client_edge_node_.clear();
  client_bandwidth_.clear();
  edge_bandwidth_.clear();

  //只输入可以使用的边缘节点信息，并利用信息赋值
  edge_node_ = data_->GetAvailableEdgeNode(day);

  edge_node_v_.clear();

  std::unordered_map<std::string, int> edge_node_seq;

  for (std::string edge : edge_node_) {
    edge_client_node_[edge] = data_->GetEdgeClient(edge);
    edge_node_v_.emplace_back(edge);
    edge_node_seq[edge] = data->GetEdgeNodeOrder(edge);
    edge_bandwidth_[edge] = 0;
  }

  //排序获得边缘节点顺序
  sort(edge_node_v_.begin(), edge_node_v_.end(),
       [&](const std::string &a, const std::string &b) {
         return edge_node_seq[a] < edge_node_seq[b];
       });

  //赋值客户节点信息，并且关联的客户节点为可以使用的边缘节点
  client_node_ = data_->GetClientSet();
  client_node_v_.clear();

  std::unordered_map<std::string, int> client_node_seq;

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
    client_node_v_.emplace_back(client);
    client_node_seq[client] = data_->GetClientNodeOrder(client);
  }

  //排序获得客户节点顺序
  sort(client_node_v_.begin(), client_node_v_.end(),
       [&](const std::string &a, const std::string &b) {
         return client_node_seq[a] < client_node_seq[b];
       });

  days_ = day;
}
void ClientDayDistribution::DistributeBalanced() {
  std::vector<std::string> client_order_ = client_node_v_;

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

    //将边缘节点剩余流量从大到小排序
    sort(connect_edge.begin(), connect_edge.end(),
         [&](std::string &a, std::string &b) {
           return data_->GetEdgeBandwidthLimit(a) - edge_bandwidth_[a] >
                  data_->GetEdgeBandwidthLimit(b) - edge_bandwidth_[b];
         });

    //对超限流量做一次尽力而为分配
    for (int i = 0; i < n; i++) {
      std::string edge = connect_edge[i];
      if (leave_bandwidth == 0) break;
      int edge_bandwidth_limit = data_->GetEdgeBandwidthLimit(edge);
      if (edge_bandwidth_limit - edge_bandwidth_[edge] >= leave_bandwidth) {
        edge_bandwidth_[edge] += leave_bandwidth;
        data_->AddDistribution(days_, client, edge, leave_bandwidth);
        leave_bandwidth = 0;
        break;
      } else {
        data_->AddDistribution(days_, client, edge,
                               edge_bandwidth_limit - edge_bandwidth_[edge]);
        leave_bandwidth -= edge_bandwidth_limit - edge_bandwidth_[edge];
        edge_bandwidth_[edge] = edge_bandwidth_limit;
      }
    }
    client_bandwidth_[client] = 0;
  }
}

void ClientDayDistribution::DistributeMaxBandwidth() {
  std::vector<std::string> client_order_ = client_node_v_;

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
           return data_->GetEdgeNode(a)->GetBandwidth() >
                  data_->GetEdgeNode(b)->GetBandwidth();
         });

    for (auto &edge_name : connect_edge) {
      if (client_bandwidth_[client] == 0) break;
      int remain =
          data_->GetEdgeBandwidthLimit(edge_name) - edge_bandwidth_[edge_name];
      if (remain > 0) {
        int occupation = std::min(client_bandwidth_[client], remain);
        edge_bandwidth_[edge_name] += occupation;
        client_bandwidth_[client] -= occupation;
        data_->AddDistribution(days_, client, edge_name, occupation);
      }
    }
  }
}

//全分配
void ClientDayDistribution::DistributeAllIn() {
  std::vector<std::string> client_order = client_node_v_;

  for (std::string client : client_order) {
    if (client_bandwidth_[client] == 0) continue;
    //获得一个该客户节点连接的边缘节点序列，按照已更新的边缘节点序排序
    std::vector<std::string> connect_edge;

    for (auto &p : client_edge_node_[client]) {
      connect_edge.emplace_back(p);
    }
    sort(connect_edge.begin(), connect_edge.end(),
         [&](std::string &a, std::string &b) {
           return data_->GetEdgeNodeOrder(a) < data_->GetEdgeNodeOrder(b);
         });

    int leave_bandwidth = client_bandwidth_[client];

    //对每个边缘节点进行流量分配
    for (std::string edge : connect_edge) {
      if (leave_bandwidth == 0) break;
      int occupy =
          std::min(leave_bandwidth,
                   data_->GetEdgeBandwidthLimit(edge) - edge_bandwidth_[edge]);
      if (occupy == 0) continue;
      leave_bandwidth -= occupy;
      edge_bandwidth_[edge] += occupy;
      data_->AddDistribution(days_, client, edge, occupy);
    }
    client_bandwidth_[client] = 0;
  }
  //按照边缘节点当前成本更新边缘节点顺序
  std::vector<std::string> edge_list = data_->GetEdgeList();
  for (std::string &edge : edge_list) {
    data_->UpdateEdgeCost(edge, edge_bandwidth_[edge]);
  }
  //按照连接节点数为第一优先级从大到小，流量大小为第二优先级从大到小排序
  sort(edge_list.begin(), edge_list.end(), [&](std::string &a, std::string &b) {
    if (data_->GetEdgeClientNum(a) == data_->GetEdgeClientNum(b))
      return data_->GetEdgeCost(a) > data_->GetEdgeCost(b);
    else
      return data_->GetEdgeClientNum(a) > data_->GetEdgeClientNum(b);
  });
  edge_node_v_ = edge_list;
  int n = edge_list.size();
  for (int i = 0; i < n; i++) data_->SetEdgeNodeOrder(edge_list[i], i);
}

//控制阈值分配
void ClientDayDistribution::DistributeThreshold() {
  std::vector<std::string> client_order_ = client_node_v_;

  data_->ResetEdgeBand();  //重置边缘节点剩余带宽和成本
  for (std::string client : client_order_) {
    if (client_bandwidth_[client] == 0) continue;
    //获得一个该客户节点连接的边缘节点序列
    std::vector<std::string> connect_edge;
    int client_bandwidth = client_bandwidth_[client];
    int total = 0;  //不增加成本时剩余带宽
    for (auto &p : client_edge_node_[client]) {
      connect_edge.emplace_back(p);
      total += data_->GetEdgeNode(p)->GetLowCostRemain();
    }
    int n = connect_edge.size();
    //不增加成本分配一次
    if (total > client_bandwidth) {  // 可以不增加成本分配完
      sort(connect_edge.begin(), connect_edge.end(),
           [&](const std::string a, const std::string b) {
             return data_->GetEdgeNode(a)->GetLowCostRemain() >
                    data_->GetEdgeNode(b)->GetLowCostRemain();
           });
    } else {
      sort(connect_edge.begin(), connect_edge.end(),
           [&](const std::string a, const std::string b) {
             return data_->GetEdgeNode(a)->GetLowCostRemain() <
                    data_->GetEdgeNode(b)->GetLowCostRemain();
           });
    }
    int leave_bandwidth = client_bandwidth;
    for (std::string &edge : connect_edge) {
      if (leave_bandwidth == 0) break;
      int alloc = std::min(data_->GetEdgeNode(edge)->GetLowCostRemain(),
                           leave_bandwidth);
      data_->AddDistribution(days_, client, edge, alloc);
      leave_bandwidth -= alloc;
      data_->GetEdgeNode(edge)->DecRemain(alloc);
    }
    //现在分配必须增加成本（最好按照T天接受请求的3/4分位数来排序）这里暂时平均放置
    sort(connect_edge.begin(), connect_edge.end(),
         [&](const std::string a, const std::string b) {
           return data_->GetEdgeNode(a)->GetRemain() <
                  data_->GetEdgeNode(b)->GetRemain();
         });
    int use_cnt = 0;
    for (std::string &edge : connect_edge) {
      if (leave_bandwidth == 0) break;
      int alloc = (leave_bandwidth + n - use_cnt - 1) / (n - use_cnt);
      alloc = std::min(data_->GetEdgeNode(edge)->GetRemain(), alloc);
      data_->AddDistribution(days_, client, edge, alloc);
      leave_bandwidth -= alloc;
      data_->GetEdgeNode(edge)->DecRemain(alloc);
      data_->GetEdgeNode(edge)->SetCostThreshold();
    }
    assert(leave_bandwidth == 0);
  }
}