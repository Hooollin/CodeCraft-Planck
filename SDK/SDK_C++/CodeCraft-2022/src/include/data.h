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

  //获得在第day天边缘节点edge分配给客户节点client的流量
  int GetDistribution(int &day, std::string &client, std::string &edge, std::string &stream_id);
  //获得在第day天客户节点client被分配情况
  two_string_key_int &GetDistribution(int &day,
                                      std::string &client);
  //获得第K天流量分配情况
  three_string_key_int &GetDistribution(int &day) { return distribution_[day]; }
  //设置在第day天边缘节点edge分配给客户节点client的流量为num
  void SetDistribution(int &day, std::string &client, std::string &edge, 
                       std::string &stream, int num);
  //增加在第day天边缘节点edge分配给客户节点client的stream_id和流量num
  void AddDistribution(int &day, std::string &client, std::string &edge,
                       std::string &stream_id, int num);

  //设置第day天的可用边缘节点集合为edge
  void SetAvailableEdgeNode(int &day,
                            std::unordered_set<std::string> &edge_nodes);
  //增加edge＿node到第day天的可用边缘节点集合
  void AddAvailableEdgeNode(int &day, std::string &edge_node);
  //第day天的可用边缘节点集合删除edge_node节点
  void DelAvailableEdgeNode(int &day, std::string &edge_node);
  //获得第day填的可用边缘节点集合
  std::unordered_set<std::string> &GetAvailableEdgeNode(int &day);

  //获得Edge边缘节点连接客户节点的集合
  std::unordered_set<std::string> &GetEdgeClient(std::string &edge);
  //获得Client客户节点连接边缘节点的集合
  std::unordered_set<std::string> &GetClientEdge(std::string &client);

  //获得Edge边缘节点连接客户节点数量
  int GetEdgeClientNum(std::string &edge);
  //获得Client客户节点连接边缘节点数量
  int GetClientEdgeNum(std::string &client);


  // 复赛接口，获得Client客户节点第day天流量需求
  std::unordered_map<std::string, int> GetClientDayDemand(int &day, std::string &client);

  //返回Edge边缘节点的带宽上限
  int &GetEdgeBandwidthLimit(std::string &edge);

  //输出client数据
  void ClientToString(std::string &client);
  //输出edge数据
  void EdgeToString(std::string &edge);

  //设置边缘节点顺序
  void SetEdgeNodeOrder(std::string &edge, int &order);
  //设置客户节点顺序
  void SetClientNodeOrder(std::string &client, int &order);
  //获得边缘节点顺序
  int &GetEdgeNodeOrder(std::string &edge);
  //获得客户节点顺序
  int &GetClientNodeOrder(std::string &client);

  //设置总天数,并对数组初始化
  void SetAllDays(int days);
  //获得总天数
  int &GetAllDays() { return alldays_; }

  //获得边缘节点列表
  std::vector<std::string> GetEdgeList();
  //获得客户节点列表
  std::vector<std::string> GetClientList();

  //获得客户节点集合
  std::unordered_set<std::string> GetClientSet();
  //获得边缘节点集合
  std::unordered_set<std::string> GetEdgeSet();

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
   //分配情况,distribution[day][client][edge][stream_id]
  std::vector<
      std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, int>>>>
      distribution_;
  //可用节点情况 available_edge_node_[day]
  std::vector<std::unordered_set<std::string>> available_edge_node_;
  //边缘节点的顺序,value是其在节点中的排序值
  std::unordered_map<std::string, int> edge_order_;
  //遍历客户节点的顺序,value是其在节点中的排序值
  std::unordered_map<std::string, int> client_order_;
  //处理每一天的顺序
  std::vector<int> days_order_;
  //总天数
  int alldays_;
  //边缘节点成本（使用最大带宽量）
  std::unordered_map<std::string, int> edge_cost_;
};

void OutputTwoStringKeyInt(two_string_key_int &data);