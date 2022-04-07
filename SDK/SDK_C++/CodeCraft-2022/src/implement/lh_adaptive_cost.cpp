#include "lh_adaptive_cost.h"

#include <queue>

void AdaptiveCost::Distribute() {
    DoAdaptiveUpdate();
}

void AdaptiveCost::DoAdaptiveUpdate() {
    std::vector<std::unordered_map<std::string, std::unordered_set<std::string>> >
        edge_serve_clients(allday_);//节点连接的客户

    std::vector<std::unordered_map<std::string, int> > 
        client_stream(allday_);//客户每天的请求流
    
    for(int day = 0; day < allday_; day ++) {
        for(std::string edge: data_->GetAvailableEdgeNode(day)) {
            edge_serve_clients[day][edge] = data_->GetEdgeClient(edge);
        }
        for(std::string client: data_->GetClientSet()) {
            client_stream[day] = data_->GetClientDayRemainingDemand(day, client);
        }
    }
    auto edge_node = edge_node_;
    sort(edge_node.begin(), edge_node.end(), [&](std::string& a, std::string&b){
        // if(data_->GetEdgeClientNum(a) == data_->GetEdgeClientNum(b))
        return data_->GetEdgeBandwidthLimit(a) < data_->GetEdgeBandwidthLimit(b);
        // return data_->GetEdgeClientNum(a) > data_->GetEdgeClientNum(b);

    });

    for(std::string& edge: edge_node_){
        //寻找边缘节点在T时刻中接受的最大请求流
        int max_stream = data_->GetBaseCost();
        for(int day = 0; day < allday_; day ++) {
            if(!edge_serve_clients[day].count(edge)) continue;
            auto cmp = [&](std::pair<std::string, int>& a, std::pair<std::string, int>& b) {
                return a.second > b.second;
            };
            std::priority_queue<std::pair<std::string, int>,
                                std::vector<std::pair<std::string, int>>,
                                decltype(cmp)> streams(cmp);
            std::unordered_set<std::string> clients = edge_serve_clients[day][edge];
            for(auto client: clients) {
                std::unordered_map<std::string, int> stream_map = 
                    client_stream[day];
                for(auto& stream: stream_map)
                    streams.emplace(stream.first, stream.second);
            }
            int deleted_stream = 0;
            while(!streams.empty()) {
                auto stream = streams.top();
                streams.pop();
                if(deleted_stream + stream.second >= max_stream) {
                    max_stream = deleted_stream + stream.second;
                    client_stream[day].erase(stream.first);
                    break;    
                }
                client_stream[day].erase(stream.first);
                deleted_stream += stream.second;
            }    
        }//已经找到最大的流
        
        //更新成本，并删除该流
        data_->UpdateEdgeCost(edge, max_stream);
        //对于其他每天的流， 从大到小删除，删除的流和不超过max_stream
        for(int day = 0; day < allday_; day ++) {
            if((!edge_serve_clients[day].count(edge))) continue;
            int deleted_stream = 0;
            auto cmp = [&](std::pair<std::string, int>& a, std::pair<std::string, int>& b) {
                return a.second < b.second;
            };
            std::priority_queue<std::pair<std::string, int>,
                                std::vector<std::pair<std::string, int>>,
                                decltype(cmp)> streams(cmp);
            std::unordered_set<std::string> clients = edge_serve_clients[day][edge];
            for(auto client: clients) {
                std::unordered_map<std::string, int> stream_map = 
                    client_stream[day];
                for(auto& stream: stream_map)
                    streams.emplace(stream.first, stream.second);
            }
            while(!streams.empty()) {
                auto stream = streams.top();
                streams.pop();
                if(deleted_stream + stream.second > max_stream) continue;
                client_stream[day].erase(stream.first);
                deleted_stream += stream.second;
            }
        }
    }
    
}