#include "lh_pre_deal.h"

#include <queue>

void LHPreDistribution::Distribute() {
    LHDistribute();
    GetDaysOrder();
    return ;
}

void LHPreDistribution::LHDistribute() {
    int percent_five_day = std::max((int)(1.0 * (data_->GetAllDays()) * 0.05), 0);
    std::unordered_map<std::string, int> picked_edge_cnt;//节点已经预处理节点数
    std::unordered_map<std::string, std::unordered_set<int> > vis_edge_day;//结点天数已经被处理
    std::vector<std::string> available_edge_node = edge_node_;
    int n = available_edge_node.size();
    int all_picked_num = n * percent_five_day;//一共需要选出的
    for(std::string& edge: available_edge_node) {
        picked_edge_cnt[edge] = 0;
    }
    //客户机连边数
    std::unordered_map<std::string, int> client_edge_num;
    for (std::string &client : client_node_) {
        client_edge_num[client] = data_->GetClientEdgeNum(client);
    }

    //边缘节点带宽
    long long max_bandwith = 0, max_serving_count = 0;
    std::unordered_map<std::string, long long> edge_bandwidth;
    for(std::string edge: available_edge_node) {
        edge_bandwidth[edge] = data_->GetEdgeBandwidthLimit(edge);
        max_bandwith = std::max(edge_bandwidth[edge], max_bandwith);
    }
    //边缘节点服务客户数
    std::unordered_map<std::string, long long> edge_servering_count;
    for(std::string edge: available_edge_node) {
        edge_servering_count[edge] = data_->GetEdgeClientNum(edge);
        max_serving_count = std::max(edge_servering_count[edge], max_serving_count);
    }
    
    //获得每日流量总需求
    days_client_bandwidth_ = std::vector<std::unordered_map<std::string, int>>(allday_);

    for (int i = 0; i < allday_; i++) {
        for (std::string &client : client_node_) {
            std::unordered_map<std::string,int> streams = data_->GetClientDayRemainingDemand(i, client);
            int demand = 0;
            for(auto it = streams.begin(); it != streams.end(); ++it){
                demand += it->second;
            }
            days_client_bandwidth_[i][client] = demand;
            assert(demand >= 0);
        }
    }

    //负载
    std::unordered_map<std::string, std::unordered_map<int, long long>> edge_day_weights;
    long long max_loading = 0;
    for(std::string edge: available_edge_node) {
        for(int day = 0; day < allday_; day ++) {
            //计算每个部分
            long long loading = 0;
            for(std::string client: data_->GetEdgeClient(edge)) {
                assert(days_client_bandwidth_[day][client] >= 0);
                loading += days_client_bandwidth_[day][client];
            }
            max_loading = std::max(loading, max_loading);
            edge_day_weights[edge][day] = loading;
        }
    }

    auto cmp = [&](std::tuple<std::string, int, double>&a, std::tuple<std::string, int, double>&b) {
        return std::get<2>(a) < std::get<2>(b);
    };
    double w1 = 0.2, w2 = 0.8, w3 = -0.03;
    //每次选取一个节点的一个时刻
    while(all_picked_num) {
        // std::cout << all_picked_num << std::endl;
        //(edge, day, weight)
        std::priority_queue<std::tuple<std::string, int, double>,
                            std::vector<std::tuple<std::string, int, double>>,
                            decltype(cmp) > priority_edges(cmp);
        
        for(auto& e: available_edge_node) {
            for(int d = 0; d < allday_; d ++) {
                if(vis_edge_day[e].count(d)) continue;//这个点已经被处理
                long long lo = edge_day_weights[e][d];
                long long band = edge_bandwidth[e];
                long long serc = edge_servering_count[e];
                double weight = w1 * lo / max_loading + w2 * band / max_bandwith + w3 * serc / max_serving_count;
                priority_edges.emplace(e, d, weight);
            }         
        }
        std::string picked_edge = "";
        int picked_day = -1;
        std::unordered_set<std::string> vis_edge;
        std::vector<int> vis_day(allday_);
        while(!priority_edges.empty() && all_picked_num) {
            auto e_d_w = priority_edges.top();
            priority_edges.pop();
            std::string e = std::get<0>(e_d_w);
            int d = std::get<1>(e_d_w);
            if(vis_edge_day[e].count(d) || picked_edge_cnt[e] >= percent_five_day) continue;
            if(vis_edge.count(e) || vis_day[d]) continue;
            vis_edge.emplace(e);
            vis_day[d] = 1;
            picked_edge = e;
            picked_day = d;
            all_picked_num --;
            //已经找出边缘节点， 开始处理分配
            int leave_bandwidth = data_->GetEdgeBandwidthLimit(picked_edge);
            assert (leave_bandwidth >= 0);
            std::vector<std::string> client_lists;
            for(std::string client: data_->GetEdgeClient(picked_edge))
                client_lists.emplace_back(client);
            //按照客户端节点连边数从小到大更新
            sort(client_lists.begin(), client_lists.end(),
            [&](const std::string &a, const std::string &b) {
            return client_edge_num[a] < client_edge_num[b];
            });
            //按照客户端节点连边数从小到大更新
            for (std::string &client : client_lists) {
                if (leave_bandwidth == 0) break;
                std::unordered_map<std::string,int> streams = data_->GetClientDayRemainingDemand(picked_day, client);
                std::vector<std::string> ordered_stream_id;
                for(auto &p : streams){
                    ordered_stream_id.emplace_back(p.first);
                }
                std::sort(ordered_stream_id.begin(), ordered_stream_id.end(), [&](std::string &a, std::string &b){
                    return streams[a] > streams[b];
                });
                int alloced = 0;
                for(auto &stream_id : ordered_stream_id){
                    int &stream_cost = streams[stream_id];
                    if(stream_cost == 0) continue;
                    if (leave_bandwidth >= stream_cost) {
                        data_->SetDistribution(picked_day, client,stream_id, picked_edge);
                        days_client_bandwidth_[picked_day][client] -= stream_cost;
                        leave_bandwidth -= stream_cost;
                        alloced += stream_cost;
                        assert(days_client_bandwidth_[picked_day][client] >= 0);
                        assert(leave_bandwidth >= 0);
                    } 
                }
                for(auto& e: data_->GetClientEdge(client)) {
                    edge_day_weights[e][picked_day] -= alloced;
                }
            }
            data_->DelAvailableEdgeNode(picked_day, picked_edge);
            vis_edge_day[picked_edge].emplace(picked_day);
            picked_edge_cnt[picked_edge] += 1;
        }
    }
    return ;
}


void LHPreDistribution::GetDaysOrder() {
  std::vector<int> client_bandwidth;
  //获取每日流量和
  std::vector<long long> days_bandwidth(allday_, 0);
  for (int i = 0; i < allday_; i++) {
    for (auto &p : days_client_bandwidth_[i]) {
      std::string client = p.first;
      int bandwidth = p.second;
      days_bandwidth[i] += bandwidth;
      client_bandwidth.emplace_back(bandwidth);
    }
  }
  //获取大流量下限
  int up_value =
      client_bandwidth[std::max((int)(client_bandwidth.size() * 0.5 - 1), 0)];
  //统计每日大流量需求数
  std::vector<int> days_big(allday_, 0);
  for (int i = 0; i < allday_; i++) {
    for (auto &p : days_client_bandwidth_[i]) {
      std::string client = p.first;
      int bandwidth = p.second;
      if (bandwidth >= up_value) days_big[i]++;
    }
  }
  std::vector<int> days_order(allday_);
  //先按大流量需求数排序，若需求相同按流量和排序
  for (int i = 0; i < allday_; i++) days_order[i] = i;
  std::sort(days_order.begin(), days_order.end(),
            [&](const int &a, const int &b) {
              return (days_big[a] == days_big[b])
                         ? (days_bandwidth[a] < days_bandwidth[b])
                         : (days_big[a] < days_big[b]);
            });
  data_->SetDaysOrder(days_order);
  return ;
}