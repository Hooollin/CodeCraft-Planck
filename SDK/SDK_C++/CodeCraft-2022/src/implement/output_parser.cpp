
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
      // <edge, <stream_id, num>> 
      two_string_key_int distributions =
          data_->GetDistribution(i, client);
      for(auto it = distributions.begin(); it != distributions.end(); ++it){
        if (flag) ofs_ << ",";
        auto edge = it->first;
        if(it->second.size() > 0){
          ofs_ << "<" << edge;
        }
        for (auto &distribution : it->second) {
          std::string stream_id = distribution.first;
          int value = distribution.second;
          if (value == 0) continue;
          ofs_ << "," << stream_id;
        }
        ofs_ << ">";
        flag = true;
      }
      ofs_ << std::endl;
    }
  }
  ofs_.close();
}