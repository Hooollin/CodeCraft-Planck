#include "lhl_strategy.h"

void LHLStrategy::Init() {
  // 初始化
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  for (auto edge : available_edge_node) {
    edge_node_remain_[edge] = data_->GetEdgeBandwidthLimit(edge);
  }
  return;
}

void LHLStrategy::Distribute() {
  Init();
  // AverageStrategy();
   BaselineStrategy();
  // ModifiedBaselineStrategy();
}

std::vector<std::pair<std::string, int>> LHLStrategy::ApplyStreamsToEdge(std::vector<std::pair<std::string, int>> all_streams, std::string client, std::vector<std::string> edges){
  int stream_size = all_streams.size(), edge_size = edges.size();
  if(edge_size == 0) return all_streams;
  //std::cout << stream_size << " " << edge_size << std::endl;
  // 将M个stream分成N组
  // 从大到小排序，然后依次分配到 M 组中，每次往当前和最小的组最后添加
  std::vector<std::vector<std::pair<std::string, int>>> buckets(
      edge_size, std::vector<std::pair<std::string, int>>());
  // 小根堆 <sum, idx>
  std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                      std::greater<std::pair<int, int>>>
      pq;
  for (int i = 0; i < edge_size; ++i) {
    pq.push({0, i});
  }

  sort(all_streams.begin(), all_streams.end(),
        [&](std::pair<std::string, int> &a, std::pair<std::string, int> &b) {
          return a.second > b.second;
        });
  for (auto &p : all_streams) {
    std::pair<int, int> to_add_p = pq.top();
    pq.pop();
    buckets[to_add_p.second].push_back(p);
    to_add_p.first += p.second;
    pq.push(to_add_p);
  }
  // 从小到大
  for (auto &bucket : buckets) {
    reverse(bucket.begin(), bucket.end());
  }

  //std::set<std::pair<std::string, int>> handled;
  std::vector<int> beyond;
  for (int i = 0; i < buckets.size(); ++i) {
    beyond.push_back(buckets[i].size());
  }
  {
    int i = 0;
    for (auto edge : edges) {
      assert(i < buckets.size());
      for (int j = 0; j < buckets[i].size(); ++j) {
        std::string stream_id = buckets[i][j].first;
        int stream_demand = buckets[i][j].second;
        if (edge_node_remain_[edge] >= stream_demand) {
          //assert(handled.find({stream_id, stream_demand}) == handled.end());
          edge_node_remain_[edge] -= stream_demand;
          data_->SetDistribution(days_, client, stream_id, edge);
          //handled.insert({stream_id, stream_demand});
        } else {
          // j及j以后的还未被处理
          beyond[i] = j;
          break;
        }
      }
      ++i;
    }
  }
  std::vector<std::pair<std::string, int>> unhandled;
  {
    int k = 0;
    // 处理还未被处理的流量
    for (int i = 0; i < beyond.size(); ++i) {
      for (int j = beyond[i]; j < buckets[i].size(); ++j) {
        std::string stream_id = buckets[i][j].first;
        int stream_demand = buckets[i][j].second;
        int last_k = k;
        while(true){
          if(last_k == k){
            unhandled.push_back(buckets[i][j]);
            break;
          }
          std::string &edge = edges[k % (int)edges.size()];
          bool good = false;
          if(edge_node_remain_[edge] >= stream_demand) {
            //assert(handled.find({stream_id, stream_demand}) == handled.end());
            edge_node_remain_[edge] -= stream_demand;
            data_->SetDistribution(days_, client, stream_id, edge);
            //handled.insert({stream_id, stream_demand});
            good = true;
          }
          ++k;
          if(good) break;
        }
      }
      //unsigned seed =
      //    std::chrono::system_clock::now().time_since_epoch().count();
      //shuffle(extra_load_edges.begin(), extra_load_edges.end(),
      //        std::default_random_engine(seed));
    }
  }
  return unhandled;
}

void LHLStrategy::AverageStrategy() {
  auto &clients = data_->GetClientList();
  sort(clients.begin(), clients.end(), [&](std::string &a, std::string &b) {
    return data_->GetClientEdgeNum(a) < data_->GetClientEdgeNum(b);
  });

  for (auto &client : clients) {
    auto &edges = data_->GetClientEdge(client);
    std::vector<std::pair<std::string, int>> all_streams;
    {
      auto &streams = data_->GetClientDayRemainingDemand(days_, client);
      for (auto p : streams) {
        if (p.second > 0) {
          all_streams.push_back(p);
        }
      }
    }
    // 已经被预处理了
    if(all_streams.size() == 0) continue;

    std::vector<std::string> to_average_list;
    for(auto edge : edges){
      if(edge_node_remain_[edge] > 0 && data_->GetEdgeNode(edge)->GetBandwidth() != edge_node_remain_[edge]){
        to_average_list.push_back(edge);
      }
    }

    std::vector<std::pair<std::string, int>> remain_streams = ApplyStreamsToEdge(all_streams, client, to_average_list);

    std::priority_queue<std::pair<int, std::string> > remain_edge;
    for (auto &edge : edges) {
      remain_edge.emplace(edge_node_remain_[edge], edge);
    }

    for (auto &stream : remain_streams) {
      auto stream_id = stream.first;
      int cost = stream.second;
      if (cost == 0) continue;
      if(remain_edge.empty()) break;
      std::string edge = remain_edge.top().second;
      if(edge_node_remain_[edge] < cost) continue;
      remain_edge.pop();
      edge_node_remain_[edge] -= cost;
      data_->SetDistribution(days_, client, stream_id, edge);
      if(edge_node_remain_[edge] > 0) remain_edge.emplace(edge_node_remain_[edge],edge);
    }
  }
  return;
}

void LHLStrategy::BaselineStrategy() {
  std::vector<std::string> &clients = data_->GetClientList();
  sort(clients.begin(), clients.end(), [&](std::string &a, std::string &b) {
    return data_->GetClientEdgeNum(a) < data_->GetClientEdgeNum(b);
  });
  std::unordered_set<std::string> available_edge_node =
      data_->GetAvailableEdgeNode(days_);
  for (auto &client : clients) {
    auto &edges = data_->GetClientEdge(client);
    std::priority_queue<std::pair<int, std::string> > remain_edge;
    for (auto &edge : edges) {
      if (available_edge_node.find(edge) != available_edge_node.end())
        remain_edge.emplace(edge_node_remain_[edge], edge);
    }

    auto streams = data_->GetClientDayRemainingDemand(days_, client);
    for (auto &stream : streams) {
      auto stream_id = stream.first;
      int cost = stream.second;
      if (cost == 0) continue;
      if(remain_edge.empty()) break;
      std::string edge = remain_edge.top().second;
      if(edge_node_remain_[edge] < cost) continue;
      remain_edge.pop();
      edge_node_remain_[edge] -= cost;
      data_->SetDistribution(days_, client, stream_id, edge);
      if(edge_node_remain_[edge] > 0) remain_edge.emplace(edge_node_remain_[edge],edge);
    }
  }
  return;
}

void LHLStrategy::ModifiedBaselineStrategy() {
  auto &clients = data_->GetClientList();
  sort(clients.begin(), clients.end(), [&](std::string &a, std::string &b){
    return data_->GetClientEdgeNum(a) < data_->GetClientEdgeNum(b);
  });

  for(auto &client : clients){
    auto &edges = data_->GetClientEdge(client);
    std::vector<std::string> used_edges, unused_edges;
    for(auto edge : edges){
      if(data_->GetEdgeBandwidthLimit(edge) < data_->GetEdgeNode(edge)->GetBandwidth()){
        used_edges.push_back(edge);
      }else{
        unused_edges.push_back(edge);
      }
    }
    sort(used_edges.begin(), used_edges.end(), [&](std::string &a, std::string &b){
      return data_->GetEdgeBandwidthLimit(a) > data_->GetEdgeBandwidthLimit(b);
    });
    sort(unused_edges.begin(), unused_edges.end(), [&](std::string &a, std::string &b){
      return data_->GetEdgeBandwidthLimit(a) > data_->GetEdgeBandwidthLimit(b);
    });
    std::vector<std::pair<std::string, int>> unhandled_streams;
    auto &streams = data_->GetClientDayRemainingDemand(days_, client);
    for(auto &stream : streams){
      auto stream_id = stream.first;
      int cost = stream.second;
      if(cost == 0) continue;
      bool inside = false;
      for(std::string edge : used_edges){
        if(edge_node_remain_[edge] >= cost){
          edge_node_remain_[edge] -= cost;
          data_->SetDistribution(days_, client, stream_id, edge);
          inside = true;
          break;
        }
      }
      if(!inside) unhandled_streams.push_back(stream);
      //assert(inside);
    }
    for(auto &stream : unhandled_streams){
      auto stream_id = stream.first;
      int cost = stream.second;
      if(cost == 0) continue;
      bool inside = false;
      for(std::string edge : unused_edges){
        if(edge_node_remain_[edge] >= cost){
          edge_node_remain_[edge] -= cost;
          data_->SetDistribution(days_, client, stream_id, edge);
          inside = true;
          break;
        }
      }
      assert(inside);
    }
  }
}