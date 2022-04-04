#pragma once
#include <set>
#include "client_node.h"
#include "edge_node.h"
#include "pre_deal.h"

class LHLPreDistribution : public PreDistribution {
 public:
  //初始化
  LHLPreDistribution(Data *data) : PreDistribution(data) {
    client_node_ = data_->GetClientList();
    edge_node_ = data_->GetEdgeList();
    allday_ = data_->GetAllDays();
  }

  //正式分配策略
  void Distribute() override;
  // LHL预分配策略
  void LHLDistribute();
  //获得每天处理顺序
  void GetDaysOrder();

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
