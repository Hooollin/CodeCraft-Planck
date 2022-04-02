#pragma once
#include "distribution_strategy.h"

class TestStrategy : public DayDistribution{
  public:
    TestStrategy(int days, Data *data) : DayDistribution(days, data){}

    void Distribute() override{
      // 初始化
      for(auto &edge : data_->GetEdgeList()){
        edge_node_remain_[edge] = data_->GetEdgeBandwidthLimit(edge);
      }

      auto clients = data_->GetClientList();
      sort(clients.begin(), clients.end(), [&](std::string &a, std::string &b){
        return data_->GetClientEdgeNum(a) < data_->GetClientEdgeNum(b);
      });

      for(auto &client : clients){
        auto edges = data_->GetClientEdge(client);
        std::vector<std::string> sorted_edges;
        for(auto &edge : edges){
          sorted_edges.push_back(edge);
        }
        sort(sorted_edges.begin(), sorted_edges.end(), [&](std::string &a, std::string &b){
          return data_->GetEdgeBandwidthLimit(a) > data_->GetEdgeBandwidthLimit(b);
        });
        auto streams = data_->GetClientDayDemand(days_, client);
        for(auto &stream : streams){
          auto stream_id = stream.first;
          int cost = stream.second;
          for(std::string edge : sorted_edges){
            if(edge_node_remain_[edge] >= cost){
              edge_node_remain_[edge] -= cost;
              data_->AddDistribution(days_, client, edge, stream_id, cost);
              break;
            }
          }
        }
      }
    }

  private:
    std::unordered_map<std::string, int> edge_node_remain_; 
};