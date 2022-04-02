#pragma once
#include <set>
#include "client_node.h"
#include "edge_node.h"
#include "pre_deal.h"

static const double UP_EDGE_BANDWIDTH_PERCENT =
    0.4;  //用以获得边缘节点顺序时使用。每天每个边缘节点最大流量之和排序后，大于该比例的流量之和算作大流量，在计数上算1。

static const double UP_CLIENT_BANDWIDTH_PERCENT =
    0.7;  //用以获得客户节点顺序时使用。每天每个客户节点流量排序后，大于该比例的流量算作大流量，在计数上算1。

static const double RARE_PERCENT = 0;

class LHKPreDistribution : public PreDistribution {
 public:
  //初始化
  LHKPreDistribution(Data *data) : PreDistribution(data) {
    client_node_ = data_->GetClientList();
    edge_node_ = data_->GetEdgeList();
    allday_ = data_->GetAllDays();
  }

  //正式分配策略
  void Distribute() override;
  // LHK预分配策略
  void LHKDistribute();
  // LHL预分配策略
  void LHLDistribute();
  //均衡预分配策略
  void BalancedDistribute();
  //获取边缘节点顺序
  void GetEdgeOrder();
  //获取客户节点排序
  void GetClientOrder();
  //获得每天处理顺序
  void GetDaysOrder();
  //二分获取边缘节点分配给客户节点的最大平均流量
  int GetAvangeBandwidth(std::vector<std::string> &client_lists, int &bandwidth,
                         int &deal_day);

 private:
  //总天数
  int allday_;
  //边缘节点列表
  std::vector<std::string> edge_node_;
  //客户节点列表
  std::vector<std::string> client_node_;
  //剩余客户节点需满足流量
  std::vector<std::unordered_map<std::string, int> > days_client_bandwidth_;
};
