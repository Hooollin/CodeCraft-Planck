#pragma once

#include <algorithm>
#include <climits>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "client_node.h"
#include "distribution_strategy.h"
#include "edge_node.h"

class ClientDayDistribution : public DayDistribution {
 public:
  ClientDayDistribution(int &day, Data *data);
  //实际分配策略
  void Distribute() override;
  //服务器均衡分配策略
  void DistributeBalanced();
  //服务器均衡扩展成本策略
  void DistributeAddCostBalanced();
  //集中在几台服务器的全分配策略
  void DistributeAllIn();
  void DistributeMaxBandwidth();
  //记录成本阈值
  void DistributeThreshold();
  //对未到达cost上限的首先进行上限均分
  void DistributeForCost();
  //带回收的均分策略
  void DistributeAverage();
  //网络流做Cost分配
  void FlowForCost();
  // balanced策略中，二分获得均衡流量值函数
  int GetAvangeBandwidthB(std::vector<std::string> &edge_lists, int &bandwidth,
                          int &day);
  // ForCost策略中，二分获得均衡流量值函数
  int GetAvangeBandwidthC(std::vector<std::string> &edge_lists, int &bandwidth,
                          int &day);
  // AddCostBalanced策略中，二分获得均衡流量值函数
  int GetAvangeBandwidthA(std::vector<std::string> &edge_lists, int &bandwidth,
                          int &day);

 private:
  //边缘节点连接的客户端节点（过滤了不在available_edge_node的边缘节点）
  one_string_key_set edge_client_node_;
  //客户端端节点连接的边缘节点（过滤了不在available_edge_node的边缘节点）
  one_string_key_set client_edge_node_;
  //已分配边缘节点流量
  std::unordered_map<std::string, int> edge_bandwidth_;
  //剩余需满足客户节点流量
  std::unordered_map<std::string, int> client_bandwidth_;
  //客户节点集合
  std::unordered_set<std::string> client_node_;
  //边缘节点集合
  std::unordered_set<std::string> edge_node_;
  //客户节点顺序序列
  std::vector<std::string> client_node_v_;
  //边缘节点顺序序列
  std::vector<std::string> edge_node_v_;
};
