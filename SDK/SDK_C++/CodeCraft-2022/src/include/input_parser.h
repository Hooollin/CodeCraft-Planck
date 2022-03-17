#pragma once
#include <bits/stdc++.h>
#include "client_node.h"
#include "edge_node.h"
class InputParser {
 public:
  InputParser(int model);

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

  std::string linux_pre_ = "..";
  std::string windows_pre_ = "../../..";
  std::string online_pre_ = "";
  std::string config_suf_ = "/data/config.ini";
  std::string demand_suf_ = "/data/demand.csv";
  std::string qos_suf_ = "/data/qos.csv";
  std::string site_bandwidth_suf_ = "/data/site_bandwidth.csv";

  std::string config_;
  std::string demand_;
  std::string qos_;
  std::string site_bandwidth_;

  std::vector<EdgeNode *> edgenode_;
  std::vector<ClientNode *> clientnode_;

  std::unordered_map<std::string, EdgeNode *> edgenode_map_;
  std::unordered_map<std::string, ClientNode *> clientnode_map_;

  std::ifstream ifs_;
};