#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
typedef std::unordered_map<std::string, std::unordered_map<std::string, int> >
    two_string_key_int;
typedef std::unordered_map<std::string, std::unordered_set<std::string> >
    one_string_key_set;

class ClientNode {
 public:
  ClientNode(std::string name) : name_(name) {}

  void AddDemand(int demand) { demand_.push_back(demand); }

  std::string GetName() { return name_; }

  void set_qos(int qos) { qos_ = qos; }

  int GetQos() { return qos_; }

  int GetDemand(int day) { return demand_[day]; }

  int GetDays() { return demand_.size(); }

  void AddAvailableEdgeNode(std::string name) {
    available_edgenode.insert(name);
  }

  std::unordered_set<std::string> GetAvailableEdgeNode() {
    return available_edgenode;
  };

  std::string ToString();

 private:
  std::string name_;                                   //客户节点名
  std::vector<int> demand_;                            //每天带宽需求
  std::unordered_set<std::string> available_edgenode;  //可用边缘节点集
  int qos_;                                            //延迟上限
};

class ClientDistribution {
 public:
 private:
  std::string name_;  //客户节点名
  std::vector<std::unordered_map<std::string, int> >
      day_bandwidth_distribution_;  //每日客户节点被边缘节点分配带宽大小
};