#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "data.h"

typedef std::unordered_map<std::string, std::unordered_map<std::string, int> >
    two_string_key_int;
typedef std::unordered_map<std::string, std::unordered_set<std::string> >
    one_string_key_set;

class OutputParser {
 public:
  OutputParser(int &model,Data *data);

  void StandradOutput();

 private:

  std::string linux_pre_ = "..";
  std::string windows_pre_ = "../../..";
  std::string online_pre_ = "";
  std::string solution_suf_ = "/output/solution.txt";
  std::string solution_;

  Data *data_;
  int day_;
  std::ofstream ofs_;

};
