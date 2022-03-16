#pragma once
#include <cassert>
#include <sstream>
#include <string>
#include <vector>

class EdgeNode;
class ClientNode {
 public:
  ClientNode(std::string name) : name_(name) {}

  int GetDemand(int k) { return demand_[k]; }

  void AddDemand(int demand) { demand_.push_back(demand); }

  std::string GetName() { return name_; }

  void set_qos(int qos) { qos_ = qos; }

  int GetQos() { return qos_; }

  void AddAvailableEdgeNode(std::string name, EdgeNode *edgenode) {
    available_edgenode_map_[name] = edgenode;
  }

  EdgeNode *GetEdgeNode(const std::string &name) {
    assert(available_edgenode_map_.find(name) != available_edgenode_map_.end());
    return available_edgenode_map_[name];
  }

  std::string ToString() {
    std::ostringstream oss;
    oss << "{\n"
        << "ClientNode: " << name_ << ",\n";
    oss << "demands: " << demand_.size() << " [\n";
    for (int i = 0; i < demand_.size(); ++i) {
      if (i > 0 && i % 20 == 0) oss << "\n";
      oss << demand_[i] << ", ";
    }
    oss << "\n], \n";
    oss << "available EdgeNodes: [\n";

    int count = 0;
    for (auto it = available_edgenode_map_.begin();
         it != available_edgenode_map_.end(); ++it) {
      if (count > 0 && count % 20 == 0) oss << "\n";
      oss << it->first << ", ";
      ++count;
    }
    oss << "\n], \n}";
    return oss.str();
  }
  /
 private:
  std::string name_;//客户节点名
  std::vector<int> demand_;//每天带宽需求
  std::map<std::string, EdgeNode *> available_edgenode_map_;//可用边缘节点集
  int qos_;//延迟上限
};