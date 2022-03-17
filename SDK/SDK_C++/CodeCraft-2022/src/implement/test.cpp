#include "test.h"
#include <bits/stdc++.h>

void Test::TestInputParser() {
  InputParser *parser;
  parser = new InputParser("../../../data/config.ini", "../../../data/demand.csv",
                           "../../../data/qos.csv", "../../../data/site_bandwidth.csv");
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


