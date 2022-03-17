#pragma once
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class ClientNode;
class EdgeNode {
 public:
  EdgeNode(std::string name, int bandwidth)
      : name_(name), bandwidth_(bandwidth), remain_(bandwidth) {
    limit_cnt_ = 0;
  }

  std::string GetName() { return name_; }
  void ResetRemain() { remain_ = bandwidth_; }
  int GetRemain() { return remain_; }
  int GetLimitCnt() { return limit_cnt_; }
  void IncLimitCnt() { limit_cnt_++; }
  void DecLimitCnt() { limit_cnt_--; }
  void IncRemain(int band) { remain_ += band; }
  void DecRemain(int band) { remain_ -= band; }
  void AddServingClientNode(std::string name, ClientNode *clientnode) {
    serving_clientnode_map_[name] = clientnode;
  }

  int GetServingClientNodeCount() { return serving_clientnode_map_.size(); }
  std::vector<ClientNode *> GetServingClientNode() {
    std::vector<ClientNode *> clients;
    for (auto [k, v] : serving_clientnode_map_) clients.push_back(v);
    return clients;
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
  std::string name_;
  int bandwidth_;
  int remain_;
  int limit_cnt_;
  std::map<std::string, ClientNode *> serving_clientnode_map_;
};