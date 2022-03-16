#pragma once
#include <iostream>
#include <map>
#include <sstream>
#include <string>

class ClientNode;
class EdgeNode {
 public:
  EdgeNode(std::string name, int bandwidth)
      : name_(name), bandwidth_(bandwidth), remain_(bandwidth) {}

  std::string GetName() { return name_; }

  void AddServingClientNode(std::string name, ClientNode *clientnode) {
    serving_clientnode_map_[name] = clientnode;
  }

  std::string ToString() {
    std::ostringstream oss;
    oss << "{\n"
        << "EdgeNode: " << name_ << ", bandwitdh: " << bandwidth_
        << ", remain: " << remain_ << ",\n";
    oss << "Serving ClientNodes: [\n";

    int count = 0;
    for (auto it = serving_clientnode_map_.begin();
         it != serving_clientnode_map_.end(); ++it) {
      if (count > 0 && count % 20 == 0) oss << "\n";
      oss << it->first << ", ";
      ++count;
    }
    oss << "\n], \n}";
    return oss.str();
  }

 private:
  std::string name_;//边缘节点名
  int bandwidth_;//边缘节点带宽上限
  int remain_;

  std::map<std::string, ClientNode *> serving_clientnode_map_;
};