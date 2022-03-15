#include "InputParser.h"

std::map<std::string, EdgeNode> InputParser::GetEdgeNodeMap() {
  return edgenode_;
}

std::map<std::string, UserNode> InputParser::GetUserNodeMap() {
  return usernode_;
}

void InputParser::Parse() {
  ParseSiteBandWidthFile();
  ParseDemandFile();
  ParseQosFile();
  ReadConfigFile();
}

void InputParser::ParseDemandFile() {
  ifs_.open(demand_, std::ios::in, 0);
  std::string header_line, normal_line;
  ifs_ >> header_line;
}

void InputParser::ParseDemandFile() {}

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