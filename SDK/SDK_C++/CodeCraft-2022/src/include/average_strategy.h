#pragma once

#include <iostream>
#include <map>
#include <vector>
#include "strategy.h"

class AverageStrategy : public Strategy {
 public:
  AverageStrategy(InputParser *input_parser, OutputParser *output_parser)
      : Strategy(input_parser, output_parser) {}

  void Init() {}

  void HandleAllTimes() {
    for (int T = 0; T < GetInputParser()->GetT(); T++) {
      GetInputParser()->ResetEdgeNode();
      HandleOneTimes(T);
    }
  }

  void CheckResult() {
    for (int T = 0; T < GetInputParser()->GetT(); T++) {
      for (auto custome : GetInputParser()->GetClientNameList()) {
        int alloc_band = 0;
        for (auto site : GetInputParser()->GetEdgeNameList()) {
          alloc_band += GetOutputParser()->GetAllocedBand(T, custome, site);
        }
        std::cout << T << ": "
                  << "<" << custome << ", " << alloc_band << ">, ";
      }
      std::cout << std::endl;
    }
  }

 private:
  //得到用户的处理顺序
  std::vector<std::string> GetCustomRank();

  //得到处理节点的顺序,被使用最大限制贷款次数最少且服务用户最多
  std::vector<std::string> GetSiteRank();

  void HandleOneSite(const std::string &site, int T);

  void HandleOneCustome(const std::string &custome, int T);

  void HandleOneCustomeAndCustom(EdgeNode *site, ClientNode *custome, int T,
                                 int demand);

  //处理T时刻
  void HandleOneTimes(int T);
};
