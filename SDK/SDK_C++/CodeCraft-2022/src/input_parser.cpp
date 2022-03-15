#include "input_parser.h"
#include "client_node.h"
#include "edge_node.h"

std::map<std::string, EdgeNode *> InputParser::GetEdgeNodeMap() {
  return edgenode_map_;
}

std::map<std::string, ClientNode *> InputParser::GetClientNodeMap() {
  return clientnode_map_;
}

void InputParser::Parse() {
  ParseSiteBandWidthFile();
  ParseDemandFile();
  ParseQosFile();
  ReadConfigFile();
}

void InputParser::ParseDemandFile() {
  ifs_.open(demand_, std::ios::in);
  std::string header_line, normal_line;
  std::vector<std::string> splitted_header_line, splitted_normal_line;

  ifs_ >> header_line;
  SplitString(header_line, ',', splitted_header_line);
  for (int i = 1; i < splitted_header_line.size(); ++i) {
    clientnode_.push_back(new ClientNode(splitted_header_line[i]));
    clientnode_map_[splitted_header_line[i]] = clientnode_.back();
  }

  while (ifs_ >> normal_line) {
    SplitString(normal_line, ',', splitted_normal_line);
    for (int i = 1; i < splitted_normal_line.size(); ++i) {
      clientnode_[i - 1]->addDemand(stoi(splitted_normal_line[i]));
    }
    splitted_normal_line.clear();
  }
}

void InputParser::ReadConfigFile() {}

void InputParser::ParseSiteBandWidthFile() {}

void InputParser::SplitString(std::string &str, char splitter,
                              std::vector<std::string> &result) {
  std::string current = "";
  for (int i = 0; i < str.size(); i++) {
    if (str[i] == splitter) {
      if (current != "") {
        result.push_back(current);
        current = "";
      }
      continue;
    }
    current += str[i];
  }
  if (current.size() != 0) result.push_back(current);
}