
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
void Test::TestPreDeal() {
  InputParser *parser;
  parser = new InputParser(model_);
  parser->Parse();

  auto client_hash = parser->GetClientNodeMap();
  auto edge_hash = parser->GetEdgeNodeMap();
  int day = client_hash.begin()->second->GetDays();

  OutputParser *out_parser;
  out_parser = new OutputParser(model_,day);

  for (auto &p : client_hash) {
    out_parser->AddClient(p.first);
  }
  PreDistribution pre_deal(edge_hash, client_hash);

  pre_deal.Distribute();
  std::vector<two_string_key_int> pre_distribution = pre_deal.GetDistribution();
  std::vector<std::unordered_set<std::string> > available_edge_node =
      pre_deal.GetAvailableEdgeNode();
  std::cout<<"========================pre_deal==================="<<std::endl;
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
  std::cout<<"========================client_edge==================="<<std::endl;
  for(auto &p :  client_hash){
    std::cout<<p.first<<":";
    auto x = p.second->GetAvailableEdgeNode();
    for(auto &pp : x) std::cout<<pp<<",";
  }
  std::vector<std::unordered_map<std::string,int> > every_day_bandwidth(day);
  std::cout<<"========================everyday_status==================="<<std::endl;
  for (int i = 0; i < day; i++) {
    std::cout << "day:" << i << std::endl;
    std::unordered_map<std::string, int> edge_bandwidth;
    auto p = pre_distribution[i];
    SimplyDayDistribution dayDistribution(
        i, edge_hash, client_hash, pre_distribution[i], available_edge_node[i]);
    dayDistribution.distribute();
    p = dayDistribution.GetDistribution();
    out_parser->SetOutput(i, p);
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
    }
    for(auto p : edge_bandwidth){
      std::string name = p.first;
      int val = p.second;
      every_day_bandwidth[i][name] = val;
    }
  }
  std::cout<<"========================final_cost==================="<<std::endl;
  int ans = 0;
  int percent_95 = (0.95 * day) - (int) (0.95 * day) > 1e-6? (0.95 * day + 1) : (0.95 * day);
  for(auto p : edge_hash){
    std::string  name = p.first;
    std::vector<int> bandwidth;
    for(int i=0;i<day;i++) bandwidth.emplace_back(every_day_bandwidth[i][name]);
    sort(bandwidth.begin(),bandwidth.end());
    int cost = bandwidth[percent_95 - 1];
    std::cout<<"<"<<name<<","<<cost<<">";
    ans += cost;
  }
  std::cout<<std::endl;
  std::cout << "total:" << ans << std::endl;

  out_parser->StandradOutput();
}