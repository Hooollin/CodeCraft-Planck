#include "test.h"

int main() {
  Test test_example(2);
  test_example.TestSimplyDayDistribution();

//  InputParser *parser;
//  parser = new InputParser(3);
//  parser->Parse();
//
//  OutputParser *out_parser;
//  out_parser = new OutputParser(3);
//
//  auto client_hash = parser->GetClientNodeMap();
//  auto edge_hash = parser->GetEdgeNodeMap();
//  int day = client_hash.begin()->second->GetDays();
//
//  out_parser->SetAllDay(day);
//
//  std::unordered_map<std::string, std::unordered_map<std::string, int> >
//      empty_distribution;
//  std::unordered_set<std::string> availale_edge;
//  availale_edge.clear();
//  empty_distribution.clear();
//  for (auto &p : edge_hash) {
//    availale_edge.insert(p.first);
//  }
//
//  for(auto &p : client_hash){
//    out_parser->AddClient(p.first);
//  }
//
//  for (int i = 0; i < day; i++) {
//    SimplyDayDistribution dayDistribution(i, edge_hash, client_hash,
//                                          empty_distribution, availale_edge);
//    dayDistribution.distribute();
//    auto p = dayDistribution.GetDistribution();
//      out_parser->SetOutput(i,p);
//  }
//
//  out_parser->StandradOutput();
  return 0;
}
