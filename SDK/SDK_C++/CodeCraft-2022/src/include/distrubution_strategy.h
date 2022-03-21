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
  //边缘节点连接的客户端节点
  one_string_key_set edge_client_node_;
  //客户端端节点连接的边缘节点
  one_string_key_set client_edge_node_;
  //已分配边缘节点流量
  std::unordered_map<std::string, int> edge_bandwidth_;
  //剩余需满足客户节点流量
  std::unordered_map<std::string, int> client_bandwidth_;
  //客户节点集合
  std::unordered_set<std::string> client_node_;
  //边缘节点集合
  std::unordered_set<std::string> edge_node_;
  //今天是第几天
  int days_;
};

class ClientDayDistribution : DayDistribution {
 public:

  ClientDayDistribution(int &day, Data *data) : DayDistribution(day,data) {}
  void DistributeBalanced();
  void DistributeForMyBest();

 private:
};
