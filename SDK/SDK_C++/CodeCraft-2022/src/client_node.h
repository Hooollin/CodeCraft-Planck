#include <string>
#include <vector>

class ClientNode {
 public:
  ClientNode(std::string name) : name_(name) {}

  int GetDemand(int k) { return demand_[k]; }

  int addDemand(int demand) { demand_.push_back(demand); }

 private:
  std::string name_;
  std::vector<int> demand_;
};