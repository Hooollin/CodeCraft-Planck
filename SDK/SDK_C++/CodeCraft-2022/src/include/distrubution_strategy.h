#pragma once
#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "client_node.h"
#include "data.h"
#include "edge_node.h"
class DayDistribution {
 public:
  DayDistribution(int &day, Data *data);

 protected:
  //数据
  Data *data_;
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
  //今天是第几天
  int days_;
};

class ClientDayDistribution : DayDistribution {
 public:
  ClientDayDistribution(int &day, Data *data) : DayDistribution(day, data) {}
  //实际分配策略
  void Distribute();
  //服务器均衡分配策略
  void DistributeBalanced();
  //集中在几台服务器的全分配策略
  void DistributeAllIn();
  void DistributeMaxBandwidth();
  //记录成本阈值
  void DistributeThreshold();
  //对未到达cost上限的首先进行上限均分
  void DistributeForCost();
 private:
};
