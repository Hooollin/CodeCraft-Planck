#pragma once

#include <math.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include "client_node.h"
#include "data.h"
#include "edge_node.h"

const double UP_BANDWIDTH_PERCENT =
    0.8;  //用以获得边缘节点顺序时使用.每天每个边缘节点最大流量之和排序后，大于该比例的流量之和算作大流量，在计数上算1。

class PreDistribution {
 public:
  //初始化
  PreDistribution(Data *data) {
    data_ = data;
    client_node_ = data_->GetClientList();
    edge_node_ = data_->GetEdgeList();
    allday_ = data_->GetAllDays();
  }

  //预分配策略
  void Distribute();

  //获取边缘节点顺序
  void GetEdgeOrder();

  //获取客户节点排序
  void GetClientOrder();
 private:
  //数据
  Data *data_;
  //总天数
  int allday_;
  //边缘节点列表
  std::vector<std::string> edge_node_;
  //客户节点列表
  std::vector<std::string> client_node_;
};
