#pragma once
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, int> >>
    three_string_key_int;
typedef std::unordered_map<std::string, std::unordered_map<std::string, int> >
    two_string_key_int;
typedef std::unordered_map<std::string, std::unordered_set<std::string> >
    one_string_key_set;

class ClientNode {
 public:
  ClientNode(std::string name) : name_(name) {}

  void AddDemand(int day, std::string stream_id, int demand) { 
    while(demand_.size() <= day){
      demand_.push_back({});
    }
    demand_[day][stream_id] = demand; 
  }

  std::string GetName() { return name_; }

  void set_qos(int qos) { qos_ = qos; }

  void set_base_cost(int base_cost){ base_cost_ = base_cost; }

  int GetQos() { return qos_; }

  int GetBaseCost() { return base_cost_; }

  // 返回该节点某一天需要处理的流量，<stream_id -> demand>
  std::unordered_map<std::string, int> GetDemandByDay(int &day) {
    assert(day < demand_.size());
    return demand_[day];
  }

  int GetDays() { return demand_.size(); }

  void AddAvailableEdgeNode(std::string name) {
    available_edgenode.insert(name);
  }

  std::unordered_set<std::string> &GetAvailableEdgeNode() {
    return available_edgenode;
  };

  int GetAvailableEdgeNodeNum() { return (int)available_edgenode.size(); };

  std::string ToString();

 private:
  std::string name_;                                   //客户节点名
  std::vector<std::unordered_map<std::string, int>> demand_;                            //每天带宽需求
  std::unordered_set<std::string> available_edgenode;  //可用边缘节点集
  int qos_;                                            //延迟上限
  int base_cost_;                                      //基础成本
};