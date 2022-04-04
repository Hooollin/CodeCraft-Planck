#include "lhl_strategy.h"

void LHLStrategy::Init(){
  // 初始化
  for(auto &edge : data_->GetEdgeList()){
    edge_node_remain_[edge] = data_->GetEdgeBandwidthLimit(edge);
  }
  
  // 减去已经分配的流量
  for(auto &client : data_->GetClientList()){
    auto distributions = data_->GetDistribution(days_, client);
    for(auto &p : distributions){
      std::string stream = p.first;
      std::string edge = p.second;
      int cost = data_->GetClientStreamDemand(days_,client,stream);
      edge_node_remain_[edge] -= cost;
    }
  }
}

void LHLStrategy::Distribute(){
  Init();
  AverageStrategy();
  //BaselineStrategy();
}

void LHLStrategy::AverageStrategy(){
  auto &clients = data_->GetClientList();
  sort(clients.begin(), clients.end(), [&](std::string &a, std::string &b){
    return data_->GetClientEdgeNum(a) < data_->GetClientEdgeNum(b);
  });

  for(auto &client : clients){
    auto &edges = data_->GetClientEdge(client);
    int total_demand = 0;
    std::vector<std::pair<std::string, int>> all_streams;

    {
      auto &streams = data_->GetClientDayRemainingDemand(days_, client);
      for(auto p : streams){
        if(p.second > 0){
          all_streams.push_back(p);
          total_demand += p.second;
        }
      }
    }

    int stream_size = all_streams.size(), edge_size = edges.size();
    // 将M个stream分成N组
    // 从大到小排序，然后依次分配到 M 组中，每次往当前和最小的组最后添加
    std::vector<std::vector<std::pair<std::string, int>>> buckets(edge_size, std::vector<std::pair<std::string, int>>());
    // 小根堆 <sum, idx>
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;
    for(int i = 0; i < edges.size(); ++i){
      pq.push({0, i});
    }

    sort(all_streams.begin(), all_streams.end(), [&](std::pair<std::string, int> &a, std::pair<std::string, int> &b){
      return a.second > b.second;
    });
    for(auto &p : all_streams){
      std::pair<int, int> to_add_p = pq.top(); pq.pop();
      buckets[to_add_p.second].push_back(p);
      to_add_p.first += p.second;
      pq.push(to_add_p);
    }
    std::unordered_set<std::string> dup_stream;
    // 从小到大
    for(auto &bucket : buckets){
      reverse(bucket.begin(), bucket.end());
    }
    std::unordered_set<std::string> handled;
    std::vector<int> beyond;
    for(int i = 0; i < buckets.size(); ++i){
      beyond.push_back(buckets[i].size());
    }
    std::unordered_set<std::string> extra = edges;
    {
      int i = 0;
      for(auto edge : edges){
        for(int j = 0; j < buckets[i].size(); ++j){
          std::string stream_id = buckets[i][j].first;
          int stream_demand = buckets[i][j].second;
          if(edge_node_remain_[edge] >= stream_demand){
            assert(handled.find(stream_id) == handled.end());
            edge_node_remain_[edge] -= stream_demand;
            data_->SetDistribution(days_, client,stream_id,edge);
            handled.insert(stream_id);
          }else{
            // j及j以后的还未被处理
            beyond[i] = j;
            extra.erase(edge);
            break;
          }
        }
        ++i;
      }
    }
    std::vector<std::string> extra_load_edges;
    for(auto &edge : extra){
      extra_load_edges.push_back(edge);
    }
    // 处理还未被处理的流量
    for(int i = 0; i < beyond.size(); ++i){
      for(int j = beyond[i], k = 0; j < buckets[i].size(); ++j, ++k){
        std::string &edge = extra_load_edges[k % (int)extra_load_edges.size()];
        std::string stream_id = buckets[i][j].first;
        int stream_demand = buckets[i][j].second;
        if(edge_node_remain_[edge] >= stream_demand){
          assert(handled.find(stream_id) == handled.end());
          edge_node_remain_[edge] -= stream_demand;
          data_->SetDistribution(days_, client,stream_id,edge);
          handled.insert(stream_id);
          break;
        }
      }
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      shuffle(extra_load_edges.begin(), extra_load_edges.end(), std::default_random_engine(seed));
    }
  }
}

void LHLStrategy::BaselineStrategy(){
  auto &clients = data_->GetClientList();
  sort(clients.begin(), clients.end(), [&](std::string &a, std::string &b){
    return data_->GetClientEdgeNum(a) < data_->GetClientEdgeNum(b);
  });

  for(auto &client : clients){
    auto &edges = data_->GetClientEdge(client);
    std::vector<std::string> sorted_edges;
    for(auto &edge : edges){
      sorted_edges.push_back(edge);
    }
    sort(sorted_edges.begin(), sorted_edges.end(), [&](std::string &a, std::string &b){
      return data_->GetEdgeBandwidthLimit(a) > data_->GetEdgeBandwidthLimit(b);
    });
    auto &streams = data_->GetClientDayRemainingDemand(days_, client);
    for(auto &stream : streams){
      auto stream_id = stream.first;
      int cost = stream.second;
      if(cost == 0) continue;
      bool inside = false;
      for(std::string edge : sorted_edges){
        if(edge_node_remain_[edge] >= cost){
          edge_node_remain_[edge] -= cost;
          data_->SetDistribution(days_, client,stream_id,edge);
          inside = true;
          break;
        }
      }
      assert(inside);
    }
  }

}