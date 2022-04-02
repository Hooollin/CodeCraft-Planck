#pragma once
#include <iomanip>
#include <map>
#include "client_day_distribution.h"
#include "client_node.h"
#include "data.h"
#include "distribution_strategy.h"
#include "edge_node.h"
#include "input_parser.h"
#include "output_parser.h"
#include "pre_deal.h"
#include "pre_deal_lhk.h"
#include "test_strategy.h"

class Test {
 public:
  Test(int model);
  void TestEdgeAndClient();
  void TestPreDeal();
  void TestEverydaysDistribution();
  void TestFinalCost();
  void TestAll();

  std::string OutputPDistribution(two_string_key_int &datas, int days);
  std::string OutputEDistribution(two_string_key_int &datas, int days);

 private:
  std::string linux_pre_ = "..";
  std::string windows_pre_ = "../../..";
  std::string online_pre_ = "";
  std::string pre_deal_suf_ = "/test/pre_deal.txt";
  std::string everydays_suf_ = "/test/everydays.txt";
  std::string final_cost_suf_ = "/test/final_cost.txt";
  std::string client_and_edge_suf_ = "/test/client_and_edge.txt";

  std::string client_and_edge_;
  std::string pre_deal_;
  std::string everydays_;
  std::string final_cost_;

  int model_;
  Data data_;

  std::ofstream ofs_;
};