#include <iostream>
#include "input_parser.h"

static bool local = true;

int main() {
  InputParser *parser;
  if(local){
    parser = new InputParser("../data/config.ini", "../data/demand.csv", "../data/qos.csv", "../data/site_bandwidth.csv");
  }else{
    parser = new InputParser();
  }
  parser->Parse();
  auto cnm = parser->GetClientNodeMap();
  auto enm = parser->GetEdgeNodeMap();
  for(auto &[name, node] : cnm){
    std::cout << node->ToString() << std::endl;
  }

  for(auto &[name, node] : enm){
    std::cout << node->ToString() << std::endl;
  }
  return 0;
}
