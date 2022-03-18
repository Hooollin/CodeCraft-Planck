#pragma once

#include <iostream>
#include <map>
#include <vector>
#include "strategy.h"

class AverageStrategy : public Strategy {
 public:
  AverageStrategy(InputParser *input_parser, OutputParser *output_parser, Advisor* advisor)
      : Strategy(input_parser, output_parser, advisor) {}

  void Init() {}

  void HandleAllTimes() {
    
    for (int T = 0; T < GetInputParser()->GetT(); T++) {
      GetInputParser()->ResetEdgeNode();
      AdvisorProcess(T);
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

  void AdvisorProcess(int T);//先处理T时刻预先分配的节点

  void HandleOneCustome(const std::string &custome, int T);

  void HandleOneCustomeAndCustom(EdgeNode *site, ClientNode *custome, int T,
                                 int demand);

  //处理T时刻
  void HandleOneTimes(int T);
};
