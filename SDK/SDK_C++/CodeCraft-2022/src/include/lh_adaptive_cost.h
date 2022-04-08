#pragma once
#include <set>
#include "client_node.h"
#include "edge_node.h"
#include "pre_deal.h"

class AdaptiveCost : public PreDistribution {
public:
    AdaptiveCost(Data *data): PreDistribution(data) {
        client_node_ = data_->GetClientList();
        edge_node_ = data_->GetEdgeList();
        allday_ = data->GetAllDays();
    }
    void Distribute() override;
    //合理化每个边缘节点必须承担的成本
    void DoAdaptiveUpdate();

private:
    int allday_;
    //边缘节点列表
    std::vector<std::string> edge_node_;
    //客户节点列表
    std::vector<std::string> client_node_;
};