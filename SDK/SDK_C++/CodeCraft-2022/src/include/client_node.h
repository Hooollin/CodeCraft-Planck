#include <string>
#include <vector>
#include <cassert>

class EdgeNode;
class ClientNode {
 public:
  ClientNode(std::string name) : name_(name) {}

  int GetDemand(int k) { return demand_[k]; }

  int AddDemand(int demand) { demand_.push_back(demand); }

  std::string GetName(){
    return name_;
  }

  void set_qos(int qos){
    qos_ = qos;
  }

  int GetQos(){
    return qos_;
  }

  void AddAvailableEdgeNode(EdgeNode* edgenode){
    available_edgenode_map_[edgenode->GetName()] = edgenode;
  }

  EdgeNode* GetEdgeNode(const std::string &name){
    assert(available_edgenode_map_.find(name) != available_edgenode_map_.end());
    return available_edgenode_map_[name]; 
  }

 private:
  std::string name_;
  std::vector<int> demand_;
  std::map<std::string, EdgeNode*> available_edgenode_map_;
  int qos_;
};