#include <iostream>
#include "average_strategy.h"
#include "input_parser.h"
#include "strategy.h"
#include "advisor.h"

static bool local = true;

int main() {
  InputParser *input_parser = nullptr;
  OutputParser *output_parser = nullptr;
  Advisor *advisor = nullptr;
  Strategy *st = nullptr;
  if (local) {
    input_parser =
        new InputParser("../data/config.ini", "../data/demand.csv",
                        "../data/qos.csv", "../data/site_bandwidth.csv");
    input_parser->Init();

    output_parser = new OutputParser(
        input_parser->GetT(), input_parser->GetClientNameList(),
        input_parser->GetEdgeNameList(), "../data/solution.txt");
  } else {
    input_parser = new InputParser();
    input_parser->Init();

    output_parser = new OutputParser(
        input_parser->GetT(), input_parser->GetClientNameList(),
        input_parser->GetEdgeNameList(), "/output/solution.txt");
  }
  std::vector<ClientNode*> clients;
  std::vector<EdgeNode*> edges;

  for(auto it = input_parser->GetClientNodeMap().begin(); it != input_parser->GetClientNodeMap().end(); ++it){
    clients.push_back(it->second);
  }
  for(auto it = input_parser->GetEdgeNodeMap().begin(); it != input_parser->GetEdgeNodeMap().end(); ++it){
    edges.push_back(it->second);
  }

  advisor = new Advisor(input_parser->GetT(), clients, edges);
  advisor->Init();
  advisor->MakeOverallSuggestion();

  // // 下面循环是测试用
  // for(int i = 0; i < input_parser->GetT(); ++i){
  //   std::cout << "day: " << i << std::endl;
  //   for(auto edgenode : edges){
  //     std::vector<std::string> o;
  //     o.push_back("  " + edgenode->GetName() + ": \n");
  //     //std::cout << edgenode->GetName() << ": " << std::endl;
  //     for(auto clientnode : clients){
  //       int val = advisor->Predict(i, edgenode->GetName(), clientnode->GetName());
  //       if(val > 0){
  //         o.push_back("    -->" + clientnode->GetName() + ": " + std::to_string(val) + "\n");
  //         //std::cout << << "->" << clientnode->GetName() << ": " << val << std::endl;
  //       }
  //     }
  //     if(o.size() > 1){
  //       for(auto &str : o){
  //         std::cout << str;
  //       }
  //     }
  //   }
  // }
  st = new AverageStrategy(input_parser, output_parser, advisor);
  st->HandleAllTimes();
  // true for local debugging
  st->MakeOutput(local);
  // st->CheckResult();
  return 0;
}
