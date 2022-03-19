#include "advisor.h"

void Advisor::Init(){
  // 计算max_loading并创建LoadingNode
  for(auto edgenode : edgenodes_){
    max_loading_change_[edgenode->GetName()] = int(0.05 * T_); // maybe buggy
    total_chance_ += int(0.05 * T_);
    for(int i = 0; i < T_; ++i){
      LoadingNode *node = new LoadingNode();
      node->which_day = i;
      node->which_edge = edgenode;
      for(auto clientnode : edgenode->GetServingClientNode()){
        client_day_specified_demand_[clientnode->GetName()][i] = clientnode->GetDemand(i);
        node->loading += clientnode->GetDemand(i);
        client_to_day_specified_loading_node_[clientnode->GetName()][i].push_back(node);
      }
      max_loadings_.push_back(node);
    }
  }
}

void Advisor::KHeapifyOptimal(int k){
  auto heap_comparator = [](LoadingNode *a, LoadingNode *b){
    return a->loading < b->loading;
  };
  int base = max_loading_change_.size() / k;
  for(int i = 0; i < max_loadings_.size(); ++i){
    if(max_loading_change_[max_loadings_[i]->which_edge->GetName()] == 0){
      continue;
    }
    if(k > 0 && (i + 1) % base == 0){
      std::make_heap(max_loadings_.begin() + i, max_loadings_.end(), heap_comparator);
      --k;
    }
    LoadingNode *to_handle = max_loadings_[i];
    int curr_day = (to_handle)->which_day;
    EdgeNode *curr_edge = (to_handle)->which_edge;
    std::string edge_name = curr_edge->GetName();
    int remain = curr_edge->GetRemain();

    // 当前的edge能否超载
    if(max_loading_change_[edge_name] > 0){
      // sort clients, 按照连接edgenode数量从少到大处理
      auto serving_clients = curr_edge->GetServingClientNode();
      std::sort(serving_clients.begin(), serving_clients.end(), [](ClientNode *a, ClientNode *b){
        return a->GetAvailableEdgeNodeCount() < b->GetAvailableEdgeNodeCount();
      });

      for(auto client : serving_clients){
        if(remain == 0) break;

        std::string client_name = client->GetName();
        int current_demand = client_day_specified_demand_[client_name][curr_day];

        // 这个用户没有需求了
        if(current_demand == 0) continue;

        // 消化这一天的需求
        int real_demand = std::min(remain, current_demand);
        remain -= real_demand;
        // 可以接受它的流量（部分或全部），记录到结果中
        hint_[curr_day][edge_name][client_name] = real_demand;
        client_day_specified_demand_[client_name][curr_day] -= real_demand;
        // 修改当天与client相关节点的loading
        for(auto loadingnode : client_to_day_specified_loading_node_[client_name][curr_day]){
          loadingnode->loading -= real_demand;
        }
      }
      assert(remain >= 0);
      --max_loading_change_[edge_name];
    }
  }
}
void Advisor::FastOptimal(){
  auto heap_comparator = [](LoadingNode *a, LoadingNode *b){
    return a->loading < b->loading;
  };
  for(int i = 0; i < max_loadings_.size() && total_chance_ > 0; ++i){
    if(max_loading_change_[max_loadings_[i]->which_edge->GetName()] == 0){
      continue;
    }
    std::make_heap(max_loadings_.begin() + i, max_loadings_.end(), heap_comparator);
    LoadingNode *to_handle = max_loadings_[i];
    int curr_day = (to_handle)->which_day;
    EdgeNode *curr_edge = (to_handle)->which_edge;
    std::string edge_name = curr_edge->GetName();
    int remain = curr_edge->GetRemain();

    // 当前的edge能否超载
    if(max_loading_change_[edge_name] > 0){
      // sort clients, 按照连接edgenode数量从少到大处理
      auto serving_clients = curr_edge->GetServingClientNode();
      std::sort(serving_clients.begin(), serving_clients.end(), [](ClientNode *a, ClientNode *b){
        return a->GetAvailableEdgeNodeCount() < b->GetAvailableEdgeNodeCount();
      });

      for(auto client : serving_clients){
        if(remain == 0) break;

        std::string client_name = client->GetName();
        int current_demand = client_day_specified_demand_[client_name][curr_day];

        // 这个用户没有需求了
        if(current_demand == 0) continue;

        // 消化这一天的需求
        int real_demand = std::min(remain, current_demand);
        remain -= real_demand;
        // 可以接受它的流量（部分或全部），记录到结果中
        hint_[curr_day][edge_name][client_name] = real_demand;
        client_day_specified_demand_[client_name][curr_day] -= real_demand;
        // 修改当天与client相关节点的loading
        for(auto loadingnode : client_to_day_specified_loading_node_[client_name][curr_day]){
          loadingnode->loading -= real_demand;
        }
      }
      assert(remain >= 0);
      --max_loading_change_[edge_name];
      --total_chance_;
    }
  }
}

void Advisor::Optimal(){
  auto heap_comparator = [](LoadingNode *a, LoadingNode *b){
    return a->loading < b->loading;
  };
  for(int i = 0; i < max_loadings_.size(); ++i){
    std::make_heap(max_loadings_.begin() + i, max_loadings_.end(), heap_comparator);
    LoadingNode *to_handle = max_loadings_[i];
    int curr_day = (to_handle)->which_day;
    EdgeNode *curr_edge = (to_handle)->which_edge;
    std::string edge_name = curr_edge->GetName();
    int remain = curr_edge->GetRemain();

    // 当前的edge能否超载
    if(max_loading_change_[edge_name] > 0){
      // sort clients, 按照连接edgenode数量从少到大处理
      auto serving_clients = curr_edge->GetServingClientNode();
      std::sort(serving_clients.begin(), serving_clients.end(), [](ClientNode *a, ClientNode *b){
        return a->GetAvailableEdgeNodeCount() < b->GetAvailableEdgeNodeCount();
      });

      for(auto client : serving_clients){
        if(remain == 0) break;

        std::string client_name = client->GetName();
        int current_demand = client_day_specified_demand_[client_name][curr_day];

        // 这个用户没有需求了
        if(current_demand == 0) continue;

        // 消化这一天的需求
        int real_demand = std::min(remain, current_demand);
        remain -= real_demand;
        // 可以接受它的流量（部分或全部），记录到结果中
        hint_[curr_day][edge_name][client_name] = real_demand;
        client_day_specified_demand_[client_name][curr_day] -= real_demand;
        // 修改当天与client相关节点的loading
        for(auto loadingnode : client_to_day_specified_loading_node_[client_name][curr_day]){
          loadingnode->loading -= real_demand;
        }
      }
      assert(remain >= 0);
      --max_loading_change_[edge_name];
    }
  }
}
void Advisor::MakeOverallSuggestion(){
  //FastOptimal();
  KHeapifyOptimal(5);
}

int Advisor::Predict(int day, std::string edge_name, std::string client_name){
  return hint_[day][edge_name][client_name];
}