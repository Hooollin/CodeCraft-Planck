#include "test.h"
#include <bits/stdc++.h>

void Test::TestInputParser() {
  InputParser *parser;
  parser = new InputParser(model_);
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
  parser = new InputParser(model_);
  parser->Parse();

  auto client_hash = parser->GetClientNodeMap();
  auto edge_hash = parser->GetEdgeNodeMap();
  int day = client_hash.begin()->second->GetDays();

  std::unordered_map<std::string, std::unordered_map<std::string, int> >
      empty_distribution;
  std::unordered_set<std::string> availale_edge;
  availale_edge.clear();
  empty_distribution.clear();
  for (auto &p : edge_hash) {
    availale_edge.insert(p.first);
  }

  for (int i = 0; i < day; i++) {
    SimplyDayDistribution dayDistribution(i, edge_hash, client_hash,
                                          empty_distribution, availale_edge);
    dayDistribution.distribute();
    auto p = dayDistribution.GetDistribution();
    std::cout<<"day:"<<i<<std::endl;
    for(auto &pp : p){
      std::cout<<"client<"<<pp.first<<","<<client_hash[pp.first]->GetDemand(i)<<">:";
      int count = 0;
        for(auto &ppp : pp.second){
          std::cout<<"<"<<ppp.first<<","<<ppp.second<<">,";
          count += ppp.second;
        }
        std::cout<<count;
        if(count != client_hash[pp.first]->GetDemand(i)) std::cout<<" count false!";
      std::cout<<std::endl;
    }
  }
}
