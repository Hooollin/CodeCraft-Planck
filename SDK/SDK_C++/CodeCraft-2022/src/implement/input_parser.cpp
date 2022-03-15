#include "input_parser.h"
#include <iostream>
#include "client_node.h"
#include "edge_node.h"

std::map<std::string, EdgeNode *> &InputParser::GetEdgeNodeMap() {
  return edgenode_map_;
}

std::map<std::string, ClientNode *> &InputParser::GetClientNodeMap() {
  return clientnode_map_;
}

void InputParser::Parse() {
  ParseDemandFile();
  ParseSiteBandWidthFile();
  ParseConfigFile();
  ParseQosFile();
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
      clientnode_[i - 1]->AddDemand(stoi(splitted_normal_line[i]));
    }
    splitted_normal_line.clear();
  }

  for (auto &node : clientnode_) {
    clientnode_map_[node->GetName()] = node;
  }

  ifs_.close();
}

void InputParser::ParseConfigFile() {
  ifs_.open(config_, std::ios::in);
  std::string first_line, second_line;

  ifs_ >> first_line >> second_line;

  std::vector<std::string> splitted_second_line;
  SplitString(second_line, '=', splitted_second_line);

  int qos = stoi(splitted_second_line[1]);
  for (auto &node : clientnode_) {
    node->set_qos(qos);
  }
  ifs_.close();
}

void InputParser::ParseQosFile() {
  ifs_.open(qos_, std::ios::in);
  std::string header_line, normal_line;
  std::vector<std::string> splitted_header_line, splitted_normal_line;

  ifs_ >> header_line;
  SplitString(header_line, ',', splitted_header_line);

  while (ifs_ >> normal_line) {
    SplitString(normal_line, ',', splitted_normal_line);
    assert(splitted_normal_line.size() == splitted_header_line.size());

    std::string edgenode_name = splitted_normal_line[0];
    assert(edgenode_map_.find(edgenode_name) != edgenode_map_.end());
    EdgeNode *edgenode = edgenode_map_[edgenode_name];

    for (int i = 1; i < splitted_normal_line.size(); ++i) {
      std::string clientnode_name = splitted_header_line[i];
      assert(clientnode_map_.find(clientnode_name) != clientnode_map_.end());
      ClientNode *clientnode = clientnode_map_[clientnode_name];
      int current_qos = stoi(splitted_normal_line[i]);
      if (current_qos <= clientnode->GetQos()) {
        clientnode->AddAvailableEdgeNode(edgenode->GetName(), edgenode);
        edgenode->AddServingClientNode(clientnode->GetName(), clientnode);
      }
    }

    splitted_normal_line.clear();
  }
  ifs_.close();
}

void InputParser::ParseSiteBandWidthFile() {
  ifs_.open(site_bandwidth_, std::ios::in);
  std::string header_line, normal_line;
  std::vector<std::string> splitted_header_line, splitted_normal_line;

  // ignore header line
  ifs_ >> header_line;

  while (ifs_ >> normal_line) {
    SplitString(normal_line, ',', splitted_normal_line);
    edgenode_.push_back(
        new EdgeNode(splitted_normal_line[0], stoi(splitted_normal_line[1])));
    splitted_normal_line.clear();
  }

  for (auto &node : edgenode_) {
    edgenode_map_[node->GetName()] = node;
  }

  ifs_.close();
}

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