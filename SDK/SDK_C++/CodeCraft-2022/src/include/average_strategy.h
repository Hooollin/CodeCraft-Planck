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

 private:
  //得到用户的处理顺序
  std::vector<std::string> GetCustomRank();

  //得到处理节点的顺序,被使用最大限制贷款次数最少且服务用户最多
  std::vector<std::string> GetSiteRank();

  void HandleOneSite(const std::string &site, int T);
  void HandleOneCustome(EdgeNode *site, ClientNode *custome, int T);
  void HandleOneTimes(int T);  //处理T时刻
};
