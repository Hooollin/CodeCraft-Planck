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

  OutputParser *out_parser;
  out_parser = new OutputParser(model_);

  auto client_hash = parser->GetClientNodeMap();
  auto edge_hash = parser->GetEdgeNodeMap();
  int day = client_hash.begin()->second->GetDays();

  std::unordered_map<std::string, std::unordered_map<std::string, int> >
      empty_distribution;
  std::unordered_set<std::string> availale_edge;

  out_parser->SetAllDay(day);

  for (auto &p : edge_hash) {
    availale_edge.insert(p.first);
  }

  for (auto &p : client_hash) {
    out_parser->AddClient(p.first);
  }

  for (int i = 0; i < day; i++) {
    SimplyDayDistribution dayDistribution(i, edge_hash, client_hash,
                                          empty_distribution, availale_edge);
    dayDistribution.distribute();
    out_parser->SetOutput(i, dayDistribution.GetDistribution());
  }
  out_parser->StandradOutput();
}
