
#include "test.h"
void OutputTwoStringKeyInt(two_string_key_int &datas){
  for(auto &p : datas){
    std::string  fistkey = p.first;
    std::cout<<fistkey<<":";
    auto &sucmap = p.second;
    for(auto &pp : sucmap){
      std::cout<<"<"<<pp.first<<","<<pp.second<<">,";
    }
    std::cout<<std::endl;
  }
}
void Test::TestEdgeAndClient(){

  }
}
void Test::TestAll() {
  //定义并处理处理输入
  InputParser input_parser(model_,&data_);
  input_parser.Parse();
  TestEdgeAndClient();
  //定义输出类
  OutputParser output_parser(model_,&data_);
  //预处理，获得预处理分配
  PreDistribution pre_distribution(&data_);
  pre_distribution.Distribute();
  pre_distribution.GetEdgeOrder();
  int allday = data_.GetAllDays();

  //进行每日处理
  for (int i = 0; i < allday; i++) {
    ClientDayDistribution day_distribution(i, &data_);
    day_distribution.DistributeBalanced();
  }

  output_parser.StandradOutput();

  //定义并处理处理输入
  InputParser input_parser(model_,&data_);
  input_parser.Parse();
  //定义输出类
  OutputParser output_parser(model_,&data_);
  //预处理，获得预处理分配
  PreDistribution pre_distribution(&data_);
  pre_distribution.Distribute();
  int allday = data_.GetAllDays();
  std::cout<<"========================pre_deal==================="<<std::endl;
  for(int i=0;i<allday;i++){
    std::cout<<"day"<<i<<":"<<std::endl;
    auto distribution = data_.GetDistribution(i);
    OutputTwoStringKeyInt(distribution);
    std::unordered_set<std::string> available_edge_node = data_.GetAvailableEdgeNode(i);
    for(std::string p : available_edge_node)
      std::cout<<p<<" ";
    for(auto &p : distribution){
      for(auto &pp : p.second){
        std::string name = pp.first;
        assert(available_edge_node.find(name) == available_edge_node.end());
      }
    }
    std::cout<<std::endl;
  }
  std::cout<<"========================client_edge==================="<<std::endl;
  std::vector<std::string> client_list = data_.GetClientList();
  for(auto &client :  client_list){
    std::cout<<client<<":";
    auto x = data_.GetClientEdge(client);
    for(auto &pp : x) std::cout<<pp<<",";
    std::cout<<std::endl;
  }
  std::cout<<"========================everyday_status==================="<<std::endl;
  for (int i = 0; i < allday; i++) {
    std::cout << "day:" << i << std::endl;
    std::unordered_map<std::string, int> edge_bandwidth;
    ClientDayDistribution day_distribution(i, &data_);
    dayDistribution.DistributeBalanced();

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
