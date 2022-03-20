
#include "distributor.h"

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

void Distributor::DayDistribute() {
  //处理输入
  InputParser input_parser(model_);
  input_parser.Parse();

  client_node_ = input_parser.GetClientNodeMap();
  edge_node_ = input_parser.GetEdgeNodeMap();
  day_ = client_node_.begin()->second->GetDays();
  //定义输出类，并进行处理
  OutputParser output_parser(model_, day_);
  for (auto &p : client_node_) output_parser.AddClient(p.first);
  //预处理，获得预处理分配和每日可用节点
  PreDistribution pre_distribution(edge_node_, client_node_);
  pre_distribution.Distribute();

  //获得边缘节点顺序
  std::vector<std::string> edge_order = pre_distribution.GetEdgeOrder();

  day_distribution_ = pre_distribution.GetDistribution();
  available_edge_node_ = pre_distribution.GetAvailableEdgeNode();

  //进行每日处理
  for (int i = 0; i < day_; i++) {
    EdgeDayDistribution day_distribution(i, edge_node_, client_node_,
                                         day_distribution_[i],
                                         available_edge_node_[i], edge_order);

    day_distribution.DistributeForMyBest();

    two_string_key_int final_distribution = day_distribution.GetDistribution();

    output_parser.SetOutput(i, final_distribution);
  }
  output_parser.StandradOutput();
  return;
}