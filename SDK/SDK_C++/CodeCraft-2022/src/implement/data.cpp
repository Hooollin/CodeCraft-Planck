#include "data.h"

void OutputTwoStringKeyInt(two_string_key_int &data) {
  for (auto &p : data) {
    std::string fistkey = p.first;
    std::cout << fistkey << ":";
    auto &sucmap = p.second;
    for (auto &pp : sucmap) {
      std::cout << "<" << pp.first << "," << pp.second << ">,";
    }
    std::cout << std::endl;
  }
}

Data::Data() {
  alldays_ = 0;
  distribution_.clear();
  available_edge_node_.clear();
  client_order_.clear();
  edge_order_.clear();
  edge_cost_.clear();
};
void Data::SetEdgeNode(std::unordered_map<std::string, EdgeNode *> &edge_map) {
  edge_node_ = edge_map;
}

void Data::SetClientNode(
    std::unordered_map<std::string, ClientNode *> &client_map) {
  client_node_ = client_map;
}

int Data::GetDistribution(int &day, std::string &client, std::string &edge) {
  if (distribution_[day][client].find(edge) == distribution_[day][client].end())
    return 0;
  else
    return distribution_[day][client][edge];
}

std::unordered_map<std::string, int> &Data::GetDistribution(
    int &day, std::string &client) {
  return distribution_[day][client];
}

void Data::SetDistribution(int &day, std::string &client, std::string &edge,
                           int num) {
  if (num == 0)
    distribution_[day][client].erase(edge);
  else
    distribution_[day][client][edge] = num;
}

void Data::AddDistribution(int &day, std::string &client, std::string &edge,
                           int num) {
  distribution_[day][client][edge] += num;
  if (distribution_[day][client][edge] == 0)
    distribution_[day][client].erase(edge);
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

int Data::GetClientDayDemand(int &day, std::string &client) {
  assert(day < alldays_);
  assert(client_node_.find(client) != client_node_.end());
  return client_node_[client]->GetDemand(day);
}

void Data::ClientToString(std::string &client) {
  client_node_[client]->ToString();
}

void Data::EdgeToString(std::string &edge) { edge_node_[edge]->ToString(); }

int &Data::GetEdgeBandwidthLimit(std::string &edge) {
  return edge_node_[edge]->GetBandwidth();
}

std::vector<std::string> Data::GetClientList() {
  std::vector<std::string> client(0);
  client.reserve(client_node_.size());
  for (auto &p : client_node_) {
    client.emplace_back(p.first);
  }
  return client;
}

std::vector<std::string> Data::GetEdgeList() {
  std::vector<std::string> edge(0);
  edge.reserve(edge_node_.size());
  for (auto &p : edge_node_) {
    edge.emplace_back(p.first);
  }
  return edge;
}

std::unordered_set<std::string> Data::GetClientSet() {
  std::unordered_set<std::string> client;
  for (auto &p : client_node_) {
    client.emplace(p.first);
  }
  return client;
}

std::unordered_set<std::string> Data::GetEdgeSet() {
  std::unordered_set<std::string> edge;
  for (auto &p : edge_node_) {
    edge.emplace(p.first);
  }
  return edge;
}

void Data::SetEdgeNodeOrder(std::string &edge, int &order) {
  edge_order_[edge] = order;
}

void Data::SetClientNodeOrder(std::string &client, int &order) {
  client_order_[client] = order;
}

int &Data::GetEdgeNodeOrder(std::string &edge) { return edge_order_[edge]; }

int &Data::GetClientNodeOrder(std::string &client) {
  return client_order_[client];
}

void Data::SetAllDays(int days) {
  alldays_ = days;
  distribution_.clear();
  distribution_.resize(alldays_);
  available_edge_node_.clear();
  available_edge_node_.resize(alldays_);
  std::unordered_set<std::string> edge_set = GetEdgeSet();
  for (int i = 0; i < days; i++) available_edge_node_[i] = edge_set;
}

EdgeNode *Data::GetEdgeNode(std::string name) { return edge_node_[name]; }

ClientNode *Data::GetClientNode(std::string name) { return client_node_[name]; }

void Data::ResetEdgeBand() {
  for (auto &edgename : GetEdgeList()) {
    GetEdgeNode(edgename)->Reset();
  }
}

void Data::UpdateEdgeCost(std::string &edge, int &num) {
  edge_cost_[edge] = std::max(edge_cost_[edge], num);
}

int Data::GetEdgeCost(std::string &edge) { return edge_cost_[edge]; }

void Data::SetDaysOrder(std::vector<int> &days_order) {
  days_order_ = days_order;
}

std::vector<int> &Data::GetDaysOrder() { return days_order_; }