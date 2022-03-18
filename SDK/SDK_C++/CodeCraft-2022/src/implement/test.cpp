#include "test.h"
void Test::OutputTwoStringKeyInt(two_string_key_int &data){
  for(auto &p : data){
    std::string  fistkey = p.first;
    std::cout<<fistkey<<":";
    auto &sucmap = p.second;
    for(auto &pp : sucmap){
      std::cout<<"<"<<pp.first<<","<<pp.second<<">,";
    }
    std::cout<<std::endl;
  }
}
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

  out_parser->SetAllDay(day);
  two_string_key_int empty_distribution;
  std::unordered_set<std::string> availale_edge;
  availale_edge.clear();
  empty_distribution.clear();
  for (auto &p : edge_hash) {
    availale_edge.insert(p.first);
  }

  for (auto &p : client_hash) {
    out_parser->AddClient(p.first);
  }

  std::unordered_map<std::string, int> max_bandwidth;
  for (int i = 0; i < day; i++) {
    SimplyDayDistribution dayDistribution(i, edge_hash, client_hash,
                                          empty_distribution, availale_edge);
    dayDistribution.distribute();
    auto p = dayDistribution.GetDistribution();
    out_parser->SetOutput(i, p);
    std::cout << "day:" << i << std::endl;
    std::unordered_map<std::string, int> edge_bandwidth;
    for (auto &pp : p) {
      std::cout << "client<" << pp.first << ","
                << client_hash[pp.first]->GetDemand(i) << ">:";
      int count = 0;
      for (auto &ppp : pp.second) {
        std::cout << "<" << ppp.first << "," << ppp.second << ">,";
        assert(ppp.second != 0);
        edge_bandwidth[ppp.first] += ppp.second;
        count += ppp.second;
      }
      std::cout << count;
      assert(count == client_hash[pp.first]->GetDemand(i));
      std::cout << std::endl;
    }
    for (auto p : edge_bandwidth) {
      std::string edge = p.first;
      int bandwidths = p.second;
      assert(bandwidths <= edge_hash[edge]->GetBandwidth());
      max_bandwidth[edge] = std::max(max_bandwidth[edge], bandwidths);
    }
  }
  int ans = 0;
  for (auto p : max_bandwidth) {
    std::string edge = p.first;
    int mv = p.second;
    std::cout << "<" << edge << "," << mv << ">";
    ans += mv;
  }
  std::cout << "total:" << ans << std::endl;

  std::cout << client_hash.begin()->second->GetQos() << std::endl;

  out_parser->StandradOutput();
}

void Test::TestPreDeal() {
  InputParser *parser;
  parser = new InputParser(model_);
  parser->Parse();

  OutputParser *out_parser;
  out_parser = new OutputParser(model_);

  auto client_hash = parser->GetClientNodeMap();
  auto edge_hash = parser->GetEdgeNodeMap();
  int day = client_hash.begin()->second->GetDays();

  for (auto &p : client_hash) {
    out_parser->AddClient(p.first);
  }

  out_parser->SetAllDay(day);

  PreDistribution pre_deal(edge_hash, client_hash);

  pre_deal.Distribute();
  std::vector<two_string_key_int> pre_distribution = pre_deal.GetDistribution();
  std::vector<std::unordered_set<std::string> > available_edge_node =
      pre_deal.GetAvailableEdgeNode();
  for(int i=0;i<day;i++){
    std::cout<<"day"<<i<<":"<<std::endl;
    OutputTwoStringKeyInt(pre_distribution[i]);
    for(auto p : available_edge_node[i])
      std::cout<<p<<" ";
    for(auto &p : pre_distribution[i]){
      for(auto &pp : p.second){
        std::string name = pp.first;
        assert(available_edge_node[i].find(name) == available_edge_node[i].end());
      }
    }
    std::cout<<std::endl;
  }

  for(auto &p :  client_hash){
    std::cout<<p.first<<":";
    auto x = p.second->GetAvailableEdgeNode();
    for(auto &pp : x) std::cout<<pp<<",";
  }

  std::unordered_map<std::string, int> max_bandwidth;
  for (int i = 0; i < day; i++) {
    SimplyDayDistribution dayDistribution(
        i, edge_hash, client_hash, pre_distribution[i], available_edge_node[i]);
    dayDistribution.distribute();
    auto p = dayDistribution.GetDistribution();
    out_parser->SetOutput(i, p);
    std::cout << "day:" << i << std::endl;
    std::unordered_map<std::string, int> edge_bandwidth;
    for (auto &pp : p) {
      std::cout << "client<" << pp.first << ","
                << client_hash[pp.first]->GetDemand(i) << ">:";
      int count = 0;
      for (auto &ppp : pp.second) {
        std::cout << "<" << ppp.first << "," << ppp.second << ">,";
        assert(ppp.second != 0);
        edge_bandwidth[ppp.first] += ppp.second;
        count += ppp.second;
      }
      std::cout << count;
      assert(count == client_hash[pp.first]->GetDemand(i));
      std::cout << std::endl;
    }
    for (auto p : edge_bandwidth) {
      std::string edge = p.first;
      int bandwidths = p.second;
      assert(bandwidths <= edge_hash[edge]->GetBandwidth());
      max_bandwidth[edge] = std::max(max_bandwidth[edge], bandwidths);
    }
  }
  int ans = 0;
  for (auto p : max_bandwidth) {
    std::string edge = p.first;
    int mv = p.second;
    std::cout << "<" << edge << "," << mv << ">";
    ans += mv;
  }
  std::cout << "total:" << ans << std::endl;

  std::cout << client_hash.begin()->second->GetQos() << std::endl;

  out_parser->StandradOutput();
}