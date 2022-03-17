#include "test.h"
#include <bits/stdc++.h>

static void Test::TestInputParser() {
  InputParser *parser;
  parser = new InputParser("../../../data/config.ini",
                           "../../../data/demand.csv", "../../../data/qos.csv",
                           "../../../data/site_bandwidth.csv");
  parser->Parse();
  auto cnm = parser->GetClientNodeMap();
  auto enm = parser->GetEdgeNodeMap();
  for (auto &p : cnm) {
    auto node = p.second;
    std::cout << node->ToString() << std::endl;
  }

  for (auto &p : enm) {
    auto node = p.second;
    std::cout << node->ToString() << std::endl;
  }
}

void Test::TestSimplyDayDistribution() {
  InputParser *parser;
  parser = new InputParser("../../../data/config.ini",
                           "../../../data/demand.csv", "../../../data/qos.csv",
                           "../../../data/site_bandwidth.csv");
  parser->Parse();

  auto client_hash = parser->GetClientNodeMap();
  auto edge_hash = parser->GetEdgeNodeMap();
  std::unordered_map<std::string, std::unordered_map<std::string, int> >
      empty_distribution;
  std::unordered_set<std::string> availale_edge;

  for(auto p : edge_hash){
    availale_edge.insert(p.first);
  }

  for(int i=0;i<20;i++){
    SimplyDayDistribution dayDistribution(i,edge_hash,client_hash,empty_distribution,availale_edge);
    dayDistribution.distribute();
    std::unordered_map<std::string, std::unordered_map<std::string, int> >
        empty_distribution = dayDistribution.GetDistribution();
  }
}
