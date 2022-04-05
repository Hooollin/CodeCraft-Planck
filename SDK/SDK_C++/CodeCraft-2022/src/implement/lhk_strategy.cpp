#include "lhk_strategy.h"

LHKStrategy::LHKStrategy(int days, Data *data) : DayDistribution(days, data) {
  // 初始化边缘节点的剩余量
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  for (auto edge : available_edge_node) {
    edge_node_remain_[edge] = data_->GetEdgeBandwidthLimit(edge);
  }
  //获取客户节点一天的请求总量，以及连边数,以初始化客户节点遍历顺序
  client_order_ = data_->GetClientList();
  std::unordered_map<std::string, long long> client_demand, connect_number;
  for (std::string &client : client_order_) {
    std::unordered_map<std::string, int> demand =
        data_->GetClientDayRemainingDemand(days, client);
    for (auto &p : demand) {
      client_demand[client] += p.second;
    }
    std::unordered_set<std::string> &connected_edge =
        data_->GetClientEdge(client);
    for (std::string edge : connected_edge) {
      if (available_edge_node.find(edge) != available_edge_node.end()) {
        connect_number[client]++;
      }
    }
  }
  //按照连接数为第一优先级从小到大，流量和为第二优先级从大到小获得客户节点遍历顺序
  std::sort(client_order_.begin(), client_order_.end(),
            [&](std::string &a, std::string &b) {
              return connect_number[a] == connect_number[b]
                         ? client_demand[a] > client_demand[b]
                         : connect_number[a] < connect_number[b];
            });
  return ;
}

void LHKStrategy::DistributeBalanced() {
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  for(std::string &client : client_order_){
    //该客户节点连接的可用边缘节点堆，堆顶是剩余流量最大的边缘节点
    std::priority_queue<std::pair<int,std::string> > edge_bandwidth_heap;
    std::unordered_set<std::string> connected_edge = data_->GetClientEdge(client);
    //堆中加入可用连接的边缘节点
    for(std::string edge : connected_edge){
      if(available_edge_node.find(edge) != available_edge_node.end()){
        edge_bandwidth_heap.emplace(edge_node_remain_[edge],edge);
      }
    }
    //对客户节点的流带宽需求从大到小排序,获得分配流的遍历顺序
    std::unordered_map<std::string,int> client_day_stream = data_->GetClientDayRemainingDemand(days_,client);
    std::vector<std::string> stream_order;
    for(auto &p : client_day_stream){
      stream_order.emplace_back(p.first);
    }
    std::sort(stream_order.begin(),stream_order.end(),[&](const std::string &a,const std::string&b){
      return client_day_stream[a] > client_day_stream[b];
    });
    //分配流
    for(std::string &stream : stream_order){
      assert(!edge_bandwidth_heap.empty());
      int edge_bandwidth = edge_bandwidth_heap.top().first;
      std::string edge = edge_bandwidth_heap.top().second;
      int demand_bandwidth = client_day_stream[stream];
      assert(demand_bandwidth <= edge_bandwidth);
      edge_bandwidth_heap.pop();
      data_->SetDistribution(days_,client,stream,edge);
      edge_node_remain_[edge] -= demand_bandwidth;
      if(edge_node_remain_[edge] > 0) edge_bandwidth_heap.emplace(edge_node_remain_[edge],edge);
    }
  }
  //更新今日边缘节点成本
  for(auto &p : edge_node_remain_){
    std::string edge = p.first;
    int cost = data_->GetEdgeBandwidthLimit(edge) - p.second;
    data_->UpdateEdgeCost(edge,cost);
  }
  return ;
}
void LHKStrategy::Distribute(){
  //BaseCostDistribute();
  //DistributeForCost();
  DistributeBalanced();
}