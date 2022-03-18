#include <string>
#include <unordered_map>
#include <unordered_set>
#include "client_node.h"
#include "edge_node.h"

class DayDistribution {
 public:
  DayDistribution(
      int day, std::unordered_map<std::string, EdgeNode *> &edge_node,
      std::unordered_map<std::string, ClientNode *> &client_node,
      std::unordered_map<std::string, std::unordered_map<std::string, int> >
          &distribution,
      std::unordered_set<std::string> &avaliable_edge_node);

 protected:
  std::unordered_map<std::string, std::unordered_set<std::string> >
      edge_client_node_;  //边缘节点连接的客户端节点
  std::unordered_map<std::string, std::unordered_set<std::string> >
      client_edge_node_;  //客户端端节点连接的边缘节点
  std::unordered_map<std::string, std::unordered_map<std::string, int> >
      distribution_;  //当日分配情况，key1为客户节点，key2为边缘节点
  std::unordered_map<std::string, int> edge_bandwidth_;  //已分配边缘节点流量
  std::unordered_map<std::string, int> edge_up_;    //边缘节点上限流量
  std::unordered_map<std::string, int> edge_cost_;  //边缘节点95%成本
  std::unordered_map<std::string, int>
      client_bandwidth_;  //剩余需满足客户节点流量
};

class SimplyDayDistribution : DayDistribution {
 public:
  SimplyDayDistribution(
      int day, std::unordered_map<std::string, EdgeNode *> &edge_node,
      std::unordered_map<std::string, ClientNode *> &client_node,
      std::unordered_map<std::string, std::unordered_map<std::string, int> >
          &distribution,
      std::unordered_set<std::string> &avaliable_edge_node);

  std::unordered_map<std::string, std::unordered_map<std::string, int> >
  GetDistribution() {
    return distribution_;
  }

  void distribute();

 private:
  std::vector<std::string>
      client_order_;  //按照连接边缘节点数量进行的客户机排序
};