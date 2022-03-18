#pragma once
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef std::unordered_map<std::string, std::unordered_map<std::string, int> >
    two_string_key_int;
typedef std::unordered_map<std::string, std::unordered_set<std::string> >
    one_string_key_set;

class EdgeNode {
 public:
  EdgeNode(std::string name, int bandwidth)
      : name_(name), bandwidth_(bandwidth), cost_(0) {}

  std::string GetName() { return name_; }

  void AddServingClientNode(std::string name) {
    serving_clientnode.insert(name);
  }
  std::unordered_set<std::string> Getservingclientnode() {
    return serving_clientnode;
  }

  int GetBandwidth() { return bandwidth_; }

  int GetCost() { return cost_; }

  int SetCost(int cost) { cost_ = cost; }

  std::string ToString();

 private:
  std::string name_;  //边缘节点名

  int bandwidth_;  //边缘节点带宽上限

  int cost_;  //当前服务器成本

  std::unordered_set<std::string> serving_clientnode;  //连接的客户节点
};

class EdgeDistribution {
 public:
 private:
  std::string name_;  //边缘节点名

  std::vector<std::unordered_map<std::string, int> >
      day_bandwidth_distribution_;  //每日边缘节点分配给客户节点带宽大小

  std::vector<int> day_bandwidth_;  //每日分配带宽总和
};