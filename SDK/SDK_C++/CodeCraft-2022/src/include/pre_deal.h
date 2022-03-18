#pragma once

#include <algorithm>
#include "client_node.h"
#include "edge_node.h"
#include <cassert>
#include <cmath>
#include <math.h>

class PreDistribution {
 public:
  PreDistribution(std::unordered_map<std::string, EdgeNode *> edge_node,
                  std::unordered_map<std::string, ClientNode *> client_node)
      : edge_node_(edge_node),
        client_node_(client_node),
        alldays(client_node.begin()->second->GetDays()) {
    available_edge_node_.clear();
    available_edge_node_.resize(alldays);
    distribution_.clear();
    distribution_.resize(alldays);
  }

  void Distribute();
  std::vector<std::unordered_set<std::string> > GetAvailableEdgeNode() {
    return available_edge_node_;
  }

  std::vector<
      two_string_key_int >
  GetDistribution() {
    return distribution_;
  }

 private:
  int alldays;                                                 //总天数
  std::unordered_map<std::string, EdgeNode *> edge_node_;      //边缘节点
  std::unordered_map<std::string, ClientNode *> client_node_;  //客户节点
  std::vector<std::unordered_set<std::string> >
      available_edge_node_;  //每天可用边缘节点
  std::vector<
      two_string_key_int >
      distribution_;  //每天预分配 distribution_[day][client][edge]
};
