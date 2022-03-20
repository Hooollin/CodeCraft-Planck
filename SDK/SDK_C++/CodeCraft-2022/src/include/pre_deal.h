#pragma once

#include <math.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include "client_node.h"
#include "edge_node.h"

const double UP_BANDWIDTH_PERCENT =
    0.7;  //用以获得边缘节点顺序时使用.每天每个边缘节点最大流量之和排序后，大于该比例的流量之和算作大流量，在计数上算1。

class PreDistribution {
 public:
  PreDistribution(std::unordered_map<std::string, EdgeNode *> edge_node,
                  std::unordered_map<std::string, ClientNode *> client_node);

  //预分配策略
  void Distribute();

  //获取边缘节点顺序
  std::vector<std::string> GetEdgeOrder();

  std::vector<std::unordered_set<std::string> > GetAvailableEdgeNode() {
    return available_edge_node_;
  }

  std::vector<two_string_key_int> GetDistribution() { return distribution_; }
  int GetDistribution(int day, std::string client, std::string edge) {
    return distribution_[day][client][edge];
  }

  std::unordered_map<std::string, int> GetDistribution(int day,
                                                       std::string client) {
    return distribution_[day][client];
  }

  two_string_key_int GetDistribution(int day) { return distribution_[day]; }

 private:
  int alldays;                                                 //总天数
  std::unordered_map<std::string, EdgeNode *> edge_node_;      //边缘节点
  std::unordered_map<std::string, ClientNode *> client_node_;  //客户节点
  std::vector<std::unordered_set<std::string> >
      available_edge_node_;  //每天可用边缘节点
  std::vector<two_string_key_int>
      distribution_;  //每天预分配 distribution_[day][client][edge]

  std::vector<std::unordered_map<std::string, int>> days_client_bandwidth_;//每天带宽需求
  std::unordered_map<std::string, std::unordered_set<std::string>> edge_client_;//边缘节点连边情况
  std::unordered_map<std::string, std::unordered_set<std::string>> client_edge_;//客户机节点连边情况
};
