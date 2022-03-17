#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class OutputParser {
 public:
  OutputParser(int model);

  void SetClientList(std::unordered_set<std::string> client_list) {
    client_list_ = client_list;
  }

  void AddClient(std::string client) { client_list_.insert(client); }

  void SetAllDay(int day);

  void SetOutput(
      int day,
      std::unordered_map<std::string, std::unordered_map<std::string, int> >
          day_distribution);

  void SetOutput(int day, std::string client,
                 std::unordered_map<std::string, int> day_client_distribution);

  void SetOutput(int day, std::string client, std::string edge,
                 int distribution);

  void StandradOutput();

 private:
  std::string linux_pre_ = "..";
  std::string windows_pre_ = "../../..";
  std::string online_pre_ = "";
  std::string solution_suf_ = "/output/solution.txt";
  std::string solution_;

  int day_;

  std::unordered_set<std::string> client_list_;

  std::ofstream ofs_;

  std::vector<
      std::unordered_map<std::string, std::unordered_map<std::string, int> > >
      output_data_;
};
