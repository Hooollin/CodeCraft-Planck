#include "edge_node.h"
#include "client_node.h"
#include "pre_deal.h"
#include "test.h"

int main() {
  Test test_example(3);
  test_example.TestPreDeal();

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
//  PreDistribution pre_deal(edge_hash, client_hash);
//
//  pre_deal.Distribute();
//
//  std::vector<two_string_key_int> pre_distribution = pre_deal.GetDistribution();
//  std::vector<std::unordered_set<std::string> > available_edge_node =
//      pre_deal.GetAvailableEdgeNode();
//
//  for(auto &p : client_hash){
//    out_parser->AddClient(p.first);
//  }
//
//  for (int i = 0; i < day; i++) {
//    SimplyDayDistribution dayDistribution(
//        i, edge_hash, client_hash, pre_distribution[i], available_edge_node[i]);
//    dayDistribution.distribute();
//
//    auto p = dayDistribution.GetDistribution();
//      out_parser->SetOutput(i,p);
//  }
//
//  out_parser->StandradOutput();
  return 0;
}
