#include "output_parser.h"

OutputParser::OutputParser(int model) {
  assert(model >= 1 && model <= 3);
  if (model == 1) {
    solution_ = online_pre_ + solution_suf_;
  } else if (model == 2) {
    solution_ = linux_pre_ + solution_suf_;
  } else {
    solution_ = windows_pre_ + solution_suf_;
  }
  client_list_.clear();
}

void OutputParser::SetAllDay(int day) {
  day_ = day;
  output_data_.clear();
  output_data_.resize(day);
}

void OutputParser::SetOutput(
    int day,
    std::unordered_map<std::string, std::unordered_map<std::string, int> >
        day_distribution) {
  output_data_[day] = day_distribution;
}

void OutputParser::SetOutput(
    int day, std::string client,
    std::unordered_map<std::string, int> day_client_distribution) {
  output_data_[day][client] = day_client_distribution;
}

void OutputParser::SetOutput(int day, std::string client, std::string edge,
                             int distribution) {
  output_data_[day][client][edge] = distribution;
}

void OutputParser::StandradOutput() {
  ofs_.clear();
  ofs_.open(solution_, std::ios::out);
  assert(ofs_.is_open());
  for (auto &day_distribution : output_data_) {
    for (auto &client : client_list_) {
      ofs_ << client << ":";
      bool flag = false;
      for (auto &distribution : day_distribution[client]) {
        std::string edge = distribution.first;
        int distri = distribution.second;
        if (distri == 0) continue;
        if (flag) ofs_ << ",";
        ofs_ << "<" << edge << "," << distri << ">";
        flag = true;
      }
      ofs_ << std::endl;
    }
  }
}