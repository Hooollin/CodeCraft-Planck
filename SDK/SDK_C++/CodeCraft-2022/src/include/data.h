#pragma once
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "client_node.h"
#include "edge_node.h"

class Data {
 public:
  Data();

  //设置Client节点
  void SetClientNode(std::unordered_map<std::string, ClientNode *> &client_map);
  //设置Edge节点
  void SetEdgeNode(std::unordered_map<std::string, EdgeNode *> &edge_map);

  //获得在第day天客户节点的流分配给的边缘节点名
  std::string GetDistribution(int &day, std::string &client,
                              std::string &stream_id);
  //获得在第day天客户节点client被分配情况
  std::unordered_map<std::string, std::string> &GetDistribution(
      int &day, std::string &client);
  //获得第K天流量分配情况
  two_string_key_string &GetDistribution(int &day) {
    return distribution_[day];
  }
  //将第day天client的stream流分配给edge
  void SetDistribution(int &day, std::string &client, std::string &stream,
                       std::string &edge);
  //撤销第day天client的stream流分配
  void EraseDistribution(int &day, std::string &client, std::string &stream);

  //设置第day天的可用边缘节点集合为edge
  void SetAvailableEdgeNode(int &day,
                            std::unordered_set<std::string> &edge_nodes);
  //增加edge＿node到第day天的可用边缘节点集合
  void AddAvailableEdgeNode(int &day, std::string &edge_node);
  //第day天的可用边缘节点集合删除edge_node节点
  void DelAvailableEdgeNode(int &day, std::string &edge_node);
  //获得第day天的可用边缘节点集合
  std::unordered_set<std::string> &GetAvailableEdgeNode(int &day);

  //获得Edge边缘节点连接客户节点的集合
  std::unordered_set<std::string> &GetEdgeClient(std::string &edge);
  //获得Client客户节点连接边缘节点的集合
  std::unordered_set<std::string> &GetClientEdge(std::string &client);

  //获得Edge边缘节点连接客户节点数量
  int GetEdgeClientNum(std::string &edge);
  //获得Client客户节点连接边缘节点数量
  int GetClientEdgeNum(std::string &client);

  //获得Client客户节点第day天剩余流和流量
  std::unordered_map<std::string, int> &GetClientDayRemainingDemand(
      int &day, std::string &client);
  //获得Client客户节点第day天所有流和流量
  std::unordered_map<std::string, int> GetClientDayTotalDemand(
      int &day, std::string &client);
  //获得Client客户节点第day天stream流的流量
  int GetClientStreamDemand(int &day, std::string &client,
                             std::string &stream);

  //返回Edge边缘节点的带宽上限
  int &GetEdgeBandwidthLimit(std::string &edge);

  //输出client数据
  std::string ClientToString(std::string &client);
  //输出edge数据
  std::string EdgeToString(std::string &edge);

  //设置总天数,并对数组初始化
  void SetAllDays(int days);
  //获得总天数
  int &GetAllDays() { return alldays_; }

  //获得边缘节点列表
  std::vector<std::string> &GetEdgeList();
  //获得客户节点列表
  std::vector<std::string> &GetClientList();

  //获得客户节点集合
  std::unordered_set<std::string> &GetClientSet();
  //获得边缘节点集合
  std::unordered_set<std::string> &GetEdgeSet();

  //获得边缘节点
  EdgeNode *GetEdgeNode(std::string name);
  //获得客户节点
  ClientNode *GetClientNode(std::string name);

  //更新边缘节点成本
  void UpdateEdgeCost(std::string &edge, int &num);
  //获得边缘节点成本
  int GetEdgeCost(std::string &edge);

  //设置每日处理顺序
  void SetDaysOrder(std::vector<int> &days_order);
  //获得每日处理顺序
  std::vector<int> &GetDaysOrder();

 private:
  //边缘节点集合
  std::unordered_map<std::string, EdgeNode *> edge_node_;
  //客户节点集合
  std::unordered_map<std::string, ClientNode *> client_node_;
  //分配情况,distribution[day][client][stream_id] = edge
  std::vector<std::unordered_map<std::string,
                                 std::unordered_map<std::string, std::string>>>
      distribution_;
  //可用节点情况 available_edge_node_[day]
  std::vector<std::unordered_set<std::string>> available_edge_node_;
  //处理每一天的顺序
  std::vector<int> days_order_;
  //总天数
  int alldays_;
  //边缘节点成本（使用最大带宽量）
  std::unordered_map<std::string, int> edge_cost_;
  //边缘和客户节点集合，以及列表
  std::unordered_set<std::string> client_set_, edge_set_;
  std::vector<std::string> client_list_, edge_list_;
  //客户剩余未分配流及流量<day, <client, <stream_id , bandwidth> >
  std::vector<two_string_key_int> remaining_demand_;
};

void OutputTwoStringKeyInt(two_string_key_int &data);