
#include "output_parser.h"

OutputParser::OutputParser(int &model, Data *data) {
  assert(model >= 1 && model <= 3);
  if (model == 1) {
    solution_ = online_pre_ + solution_suf_;
  } else if (model == 2) {
    solution_ = linux_pre_ + solution_suf_;
  } else {
    solution_ = windows_pre_ + solution_suf_;
  }
  data_ = data;
  day_ = data->GetAllDays();
}

void OutputParser::StandradOutput() {
  ofs_.clear();
  ofs_.open(solution_, std::ios::out);
  assert(ofs_.is_open());
  std::vector<std::string> client_list = data_->GetClientList();
  for (int i = 0; i < day_; i++) {
    for (std::string client : client_list) {
      ofs_ << client << ":";
      bool flag = false;
      std::unordered_map<std::string, int> distributions =
          data_->GetDistribution(i, client);
      for (auto &distribution : distributions) {
        std::string edge = distribution.first;
        int value = distribution.second;
        if (value == 0) continue;
        if (flag) ofs_ << ",";
        ofs_ << "<" << edge << "," << value << ">";
        flag = true;
      }
      ofs_ << std::endl;
    }
  }
  ofs_.close();
}