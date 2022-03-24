
#include "test.h"

Test::Test(int model) : model_(model) {
  data_ = *(new Data);
  assert(model >= 1 && model <= 3);
  if (model == 1) {
    client_and_edge_ = everydays_ = pre_deal_ = final_cost_ = online_pre_;
  } else if (model == 2) {
    client_and_edge_ = everydays_ = pre_deal_ = final_cost_ = linux_pre_;
  } else {
    client_and_edge_ = everydays_ = pre_deal_ = final_cost_ = windows_pre_;
  }
  client_and_edge_ += client_and_edge_suf_;
  everydays_ += everydays_suf_;
  final_cost_ += final_cost_suf_;
  pre_deal_ += pre_deal_suf_;
}
std::string Test::OutputPDistribution(two_string_key_int &datas, int days) {
  std::stringstream ssm;
  for (auto &p : datas) {
    std::string firstkey = p.first;
    ssm << "<" << firstkey << "," << data_.GetClientDayDemand(days, firstkey)
        << ">"
        << ":";
    auto &sucmap = p.second;
    for (auto &pp : sucmap) {
      ssm << "<" << pp.first << "," << pp.second << ">,";
    }
    ssm << std::endl;
  }
  return ssm.str();
}

std::string Test::OutputEDistribution(two_string_key_int &datas, int days) {
  std::stringstream ssm;
  for (auto &p : datas) {
    std::string firstkey = p.first;
    int counts = 0;
    ssm << "<" << firstkey << "," << data_.GetClientDayDemand(days, firstkey)
        << ">"
        << ":";
    auto &sucmap = p.second;
    for (auto &pp : sucmap) {
      ssm << "<" << pp.first << "," << pp.second << ">,";
      counts += pp.second;
    }
    ssm << " total:" << counts;
    assert(counts == data_.GetClientDayDemand(days, firstkey));
    ssm << std::endl;
  }
  return ssm.str();
}

void Test::TestEdgeAndClient() {
  ofs_.clear();
  ofs_.open(client_and_edge_, std::ios::out);
  assert(ofs_.is_open());
  int days = data_.GetAllDays();
  std::vector<std::string> edge_node = data_.GetEdgeList();
  std::vector<std::string> client_node = data_.GetClientList();
  int edge_size = edge_node.size();
  int client_size = client_node.size();
  for (std::string edge : edge_node) {
    EdgeNode *node = data_.GetEdgeNode(edge);
    ofs_ << node->ToString() << std::endl;
    assert(data_.GetEdgeBandwidthLimit(edge) >= 0);
    assert(data_.GetEdgeClientNum(edge) >= 0 );
    assert(data_.GetEdgeClientNum(edge) <= client_size);
  }

  for (std::string client : client_node) {
    ClientNode *node = data_.GetClientNode(client);
    assert(node->GetDays() == days);
    for (int i = 0; i < days; i++) {
      assert(data_.GetClientDayDemand(i, client) >= 0);
    }
    assert(data_.GetClientEdgeNum(client) >= 0 &&
           data_.GetClientEdgeNum(client) <= edge_size);
    ofs_ << node->ToString() << std::endl;
  }
  ofs_.close();
}

void Test::TestPreDeal() {
  ofs_.clear();
  ofs_.open(pre_deal_, std::ios::out);
  assert(ofs_.is_open());

  int allday = data_.GetAllDays();
  for (int i = 0; i < allday; i++) {
    ofs_ << "day" << i << ":" << std::endl;
    auto distribution = data_.GetDistribution(i);
    ofs_ << OutputPDistribution(distribution, i);
    std::unordered_set<std::string> available_edge_node =
        data_.GetAvailableEdgeNode(i);
    ofs_ << "can use available edge node :";
    for (std::string p : available_edge_node) ofs_ << p << " ";
    ofs_ << std::endl;
    for (auto &p : distribution) {
      for (auto &pp : p.second) {
        std::string name = pp.first;
        assert(available_edge_node.find(name) == available_edge_node.end());
      }
    }
    ofs_ << std::endl;
  }
  ofs_.close();
}

void Test::TestEverydaysDistribution() {
  ofs_.clear();
  ofs_.open(everydays_, std::ios::out);
  assert(ofs_.is_open());

  int allday = data_.GetAllDays();
  std::unordered_map<std::string, int> edge_bandwidth;
  for (int i = 0; i < allday; i++) {
    ofs_ << "day" << i << ":" << std::endl;
    auto distribution = data_.GetDistribution(i);
    ofs_ << OutputEDistribution(distribution, i);
    std::unordered_set<std::string> available_edge_node =
        data_.GetAvailableEdgeNode(i);
    ofs_ << std::endl;
  }
  ofs_.close();
}

void Test::TestFinalCost() {
  ofs_.clear();
  ofs_.open(final_cost_, std::ios::out);
  assert(ofs_.is_open());

  int alldays = data_.GetAllDays();
  int percent_95 = (0.95 * alldays) - (int)(0.95 * alldays) > 1e-6
                       ? (0.95 * alldays + 1)
                       : (0.95 * alldays);
  std::unordered_map<std::string, std::vector<int> > edge_days_bandwidth;
  std::vector<std::string> edge_node = data_.GetEdgeList();
  for (std::string edge : edge_node) {
    edge_days_bandwidth[edge].clear();
    edge_days_bandwidth[edge].resize(alldays);
  }
  for (int i = 0; i < alldays; i++) {
    auto distribution = data_.GetDistribution(i);
    for (auto &p : distribution) {
      std::string client = p.first;
      for (auto &pp : p.second) {
        std::string edge = pp.first;
        edge_days_bandwidth[edge][i] += pp.second;
      }
    }
  }
  ofs_ << "edge cost :"<<std::endl;
  long long total_cost = 0;
  for (auto &p : edge_days_bandwidth) {
    std::string edge = p.first;
    std::vector<int> &bandwidth = p.second;
    sort(bandwidth.begin(), bandwidth.end());
    int cost = bandwidth[percent_95 - 1];
    ofs_ << "<" << edge << "," << cost << ">"<<std::endl;
    total_cost += cost;
  }
  ofs_ << std::endl;
  ofs_ << "total cost :" << total_cost << std::endl;
  ofs_.close();
}
void Test::TestAll() {
  //定义并处理处理输入
  InputParser input_parser(model_, &data_);
  input_parser.Parse();
  TestEdgeAndClient();
  //定义输出类
  OutputParser output_parser(model_, &data_);
  //预处理，获得预处理分配
  PreDistribution pre_distribution(&data_);
  pre_distribution.Distribute();
  pre_distribution.GetEdgeOrder();
  pre_distribution.GetClientOrder();
  pre_distribution.GetDaysOrder();
  TestPreDeal();

  int allday = data_.GetAllDays();
  std::vector<int> days_order = data_.GetDaysOrder();
  //进行每日处理
  for (int i = 0; i < allday; i++) {
    int nowaday = days_order[i];
    ClientDayDistribution day_distribution(nowaday, &data_);
    day_distribution.Distribute();
  }
  TestEverydaysDistribution();
  TestFinalCost();
  output_parser.StandradOutput();
}
