#pragma once
#include <bits/stdc++.h>
#include "client_node.h"
#include "edge_node.h"

class InputParser {
 public:
  InputParser(std::string config = "/data/config.ini",
              std::string demand = "/data/demand.csv",
              std::string qos = "/data/qos.csv",
              std::string site_bandwidth = "/data/site_bandwidth.csv")
      : config_(config),
        demand_(demand),
        qos_(qos),
        site_bandwidth_(site_bandwidth) {}

  InputParser(InputParser &) = delete;
  InputParser(InputParser &&) = delete;

  std::unordered_map<std::string, EdgeNode *> &GetEdgeNodeMap();

  std::unordered_map<std::string, ClientNode *> &GetClientNodeMap();

  void Parse();

 private:
  void ParseConfigFile();

  void ParseDemandFile();

  void ParseQosFile();

  void ParseSiteBandWidthFile();

  void SplitString(std::string &, char, std::vector<std::string> &);

  std::string config_;
  std::string demand_;
  std::string qos_;
  std::string site_bandwidth_;

  std::vector<EdgeNode *> edgenode_;
  std::vector<ClientNode *> clientnode_;

  std::unordered_map<std::string, EdgeNode *> edgenode_map_;
  std::unordered_map<std::string, ClientNode *> clientnode_map_;

  std::fstream ifs_;
};