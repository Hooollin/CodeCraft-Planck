#include "distrubution_strategy.h"
#include <bits/stdc++.h>

DayDistribution::DayDistribution(
    int day, std::unordered_map<std::string, EdgeNode *> &edge_node,
    std::unordered_map<std::string, ClientNode *> &client_node,
    std::unordered_map<std::string, std::unordered_map<std::string, int> >
        &distribution,
    std::unordered_set<std::string> &avaliable_edge_node) {
  edge_client_node.clear();
  client_edge_node.clear();
  distribution_ = distribution;
  for (auto &p : avaliable_edge_node) {
    edge_client_node[p] = edge_node[p]->Getservingclientnode();
    edge_up_[p] = edge_node[p]->GetBandwidth();
    edge_bandwith_[p] = 0;
    edge_cost_[p] = edge_node[p]->GetCost();
  }
  client_order_.clear();
  for(auto &p : client_node){
    std::string  name = p.first;
    ClientNode *client = p.second;
    std::unordered_set<std::string> tmp = client->GetAvailableEdgeNode();
    for(auto &pp : tmp){
      if(avaliable_edge_node.find(pp) != avaliable_edge_node.end()) client_edge_node[name].insert(pp);
    }
    client_bandwith_[name] = client->GetDemand(day);
    client_order_.emplace_back(name);
  }

  sort(client_order_.begin(),client_order_.end(),[&](const std::string a, const std::string b){
    return client_edge_node[a].size() < client_edge_node[b].size();
  });
}

void DayDistribution::distribute(){

}
