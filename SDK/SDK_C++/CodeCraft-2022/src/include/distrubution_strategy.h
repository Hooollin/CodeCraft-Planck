#pragma once
#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "client_node.h"
#include "edge_node.h"

class DayDistribution {
 public:
  DayDistribution(int day,
                  std::unordered_map<std::string, EdgeNode *> &edge_node,
                  std::unordered_map<std::string, ClientNode *> &client_node,
                  two_string_key_int &distribution,
                  std::unordered_set<std::string> &avaliable_edge_node);

 protected:
  one_string_key_set edge_client_node_;  //边缘节点连接的客户端节点
  one_string_key_set client_edge_node_;  //客户端端节点连接的边缘节点
  two_string_key_int distribution_;  //当日分配情况，distribution_[client][edge]
  std::unordered_map<std::string, int> edge_bandwidth_;  //已分配边缘节点流量
  std::unordered_map<std::string, int> edge_up_;  //边缘节点上限流量
  std::unordered_map<std::string, int>
      client_bandwidth_;  //剩余需满足客户节点流量
};

class ClientDayDistribution : DayDistribution {
 public:
  ClientDayDistribution(
      int day, std::unordered_map<std::string, EdgeNode *> &edge_node,
      std::unordered_map<std::string, ClientNode *> &client_node,
      two_string_key_int &distribution,
      std::unordered_set<std::string> &avaliable_edge_node);

  two_string_key_int GetDistribution() { return distribution_; }

  void DistributeBalanced();
  void DistributeForMyBest();

 private:
  std::vector<std::string>
      client_order_;  //按照连接边缘节点数量进行的客户机排序
};

class EdgeDayDistribution : DayDistribution {
 public:
  EdgeDayDistribution(
      int day, std::unordered_map<std::string, EdgeNode *> &edge_node,
      std::unordered_map<std::string, ClientNode *> &client_node,
      two_string_key_int &distribution,
      std::unordered_set<std::string> &avaliable_edge_node,
      std::vector<std::string> edge_order)
      : DayDistribution(day, edge_node, client_node, distribution,
                        avaliable_edge_node) {
    edge_order_.clear();
    for (auto &edge : edge_order)
      if (avaliable_edge_node.find(edge) != avaliable_edge_node.end())
        edge_order_.emplace_back(edge);
  }

  two_string_key_int GetDistribution() { return distribution_; }
  int GetDistribution(std::string client, std::string edge) {
    return distribution_[client][edge];
  }

  void DistributeForMyBest();

 private:
  std::vector<std::string> edge_order_;  //边缘节点排序
};