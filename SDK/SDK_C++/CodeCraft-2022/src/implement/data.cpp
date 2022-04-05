#include "data.h"

Data::Data() {
  alldays_ = 0;
  distribution_.clear();
  available_edge_node_.clear();
  edge_cost_.clear();
};

void Data::SetEdgeNode(std::unordered_map<std::string, EdgeNode *> &edge_map) {
  edge_set_.clear();
  edge_list_.clear();
  edge_node_ = edge_map;
  for (auto &p : edge_node_) {
    edge_set_.emplace(p.first);
    edge_list_.emplace_back(p.first);
  }
  return ;
}

void Data::SetClientNode(
    std::unordered_map<std::string, ClientNode *> &client_map) {
  client_set_.clear();
  client_list_.clear();
  client_node_ = client_map;
  for (auto &p : client_node_) {
    client_set_.emplace(p.first);
    client_list_.emplace_back(p.first);
  }
  return ;
}

std::string Data::GetDistribution(int &day, std::string &client,
                                  std::string &stream_id) {
  assert(distribution_[day][client].find(stream_id) !=
         distribution_[day][client].end());
  return distribution_[day][client][stream_id];
}

std::unordered_map<std::string, std::string> &Data::GetDistribution(
    int &day, std::string &client) {
  return distribution_[day][client];
}

void Data::SetDistribution(int &day, std::string &client, std::string &stream,
                           std::string &edge) {
  assert(distribution_[day][client].find(stream) ==
         distribution_[day][client].end());
  assert(remaining_demand_[day][client].find(stream) !=
         remaining_demand_[day][client].end());
  distribution_[day][client][stream] = edge;
  remaining_demand_[day][client].erase(stream);
}

void Data::EraseDistribution(int &day, std::string &client,
                             std::string &stream) {
  assert(distribution_[day][client].find(stream) !=
         distribution_[day][client].end());
  assert(remaining_demand_[day][client].find(stream) ==
         remaining_demand_[day][client].end());
  distribution_[day][client].erase(stream);
  remaining_demand_[day][client][stream] =
      client_node_[client]->GetDemandByDayStream(day, stream);
}

void Data::SetAvailableEdgeNode(int &day,
                                std::unordered_set<std::string> &edge_nodes) {
  available_edge_node_[day] = edge_nodes;
}

void Data::AddAvailableEdgeNode(int &day, std::string &edge_node) {
  available_edge_node_[day].insert(edge_node);
}

void Data::DelAvailableEdgeNode(int &day, std::string &edge_node) {
  available_edge_node_[day].erase(edge_node);
}

std::unordered_set<std::string> &Data::GetAvailableEdgeNode(int &day) {
  return available_edge_node_[day];
}

std::unordered_set<std::string> &Data::GetEdgeClient(std::string &edge) {
  return edge_node_[edge]->Getservingclientnode();
}

std::unordered_set<std::string> &Data::GetClientEdge(std::string &client) {
  return client_node_[client]->GetAvailableEdgeNode();
}

int Data::GetEdgeClientNum(std::string &edge) {
  return edge_node_[edge]->GetservingclientnodeNum();
}

int Data::GetClientEdgeNum(std::string &client) {
  return client_node_[client]->GetAvailableEdgeNodeNum();
}

std::unordered_map<std::string, int> &Data::GetClientDayRemainingDemand(
    int &day, std::string &client) {
  assert(day < alldays_);
  assert(client_node_.find(client) != client_node_.end());
  return remaining_demand_[day][client];
}

std::unordered_map<std::string, int> Data::GetClientDayTotalDemand(
    int &day, std::string &client) {
  assert(day < alldays_);
  assert(client_node_.find(client) != client_node_.end());
  return client_node_[client]->GetDemandByDay(day);
}

int Data::GetClientStreamDemand(int &day, std::string &client,
                                std::string &stream) {
  return client_node_[client]->GetDemandByDayStream(day, stream);
}

std::string Data::ClientToString(std::string &client) {
  return client_node_[client]->ToString();
}

std::string Data::EdgeToString(std::string &edge) { return edge_node_[edge]->ToString(); }

int &Data::GetEdgeBandwidthLimit(std::string &edge) {
  return edge_node_[edge]->GetBandwidth();
}

std::vector<std::string> &Data::GetClientList() { return client_list_; }

std::vector<std::string> &Data::GetEdgeList() { return edge_list_; }

std::unordered_set<std::string> &Data::GetClientSet() { return client_set_; }

std::unordered_set<std::string> &Data::GetEdgeSet() { return edge_set_; }

void Data::SetAllDays(int days) {
  alldays_ = days;
  distribution_.clear();
  distribution_.resize(alldays_);
  remaining_demand_.clear();
  remaining_demand_.resize(alldays_);

  available_edge_node_.clear();
  available_edge_node_.resize(alldays_);
  std::unordered_set<std::string> edge_set = GetEdgeSet();
  for (int i = 0; i < days; ++i) available_edge_node_[i] = edge_set;
  for (int i = 0; i < days; ++i) {
    for (auto &client : client_list_) {
      remaining_demand_[i][client] = client_node_[client]->GetDemandByDay(i);
    }
  }
}

EdgeNode *Data::GetEdgeNode(std::string name) { return edge_node_[name]; }

ClientNode *Data::GetClientNode(std::string name) { return client_node_[name]; }

void Data::UpdateEdgeCost(std::string &edge, int &num) {
  edge_cost_[edge] = std::max(edge_cost_[edge], num);
}

int Data::GetEdgeCost(std::string &edge) { return edge_cost_[edge]; }

void Data::SetDaysOrder(std::vector<int> &days_order) {
  days_order_ = days_order;
}

std::vector<int> &Data::GetDaysOrder() { return days_order_; }