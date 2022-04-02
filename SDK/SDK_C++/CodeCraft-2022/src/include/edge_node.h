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
      : name_(name), bandwidth_(bandwidth) {}

  std::string GetName() { return name_; }

  void AddServingClientNode(std::string name) {
    serving_clientnode.insert(name);
  }

  std::unordered_set<std::string> &Getservingclientnode() {
    return serving_clientnode;
  }

  int GetservingclientnodeNum() { return (int)serving_clientnode.size(); }

  int &GetBandwidth() { return bandwidth_; }

  std::string ToString();

 private:
  std::string name_;  //边缘节点名

  int bandwidth_;       //边缘节点带宽上限

  std::unordered_set<std::string> serving_clientnode;  //连接的客户节点
};