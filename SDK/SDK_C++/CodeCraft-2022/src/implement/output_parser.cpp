
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
      // 输出每个client的分配
      std::unordered_map<std::string, std::string> distributions =
          data_->GetDistribution(i, client);
      //存储每个边缘节点满足的流
      std::unordered_map<std::string,std::unordered_set<std::string> > final_distribution;
      for (auto &p : distributions) {
        final_distribution[p.second].emplace(p.first);
      }
      //输出每个边缘节点满足的流
      for(auto &p : final_distribution){
        if (flag) ofs_ << ",";
        ofs_ << "<" << p.first ;
        for(std::string stream : p.second){
          ofs_ <<"," << stream;
        }
        ofs_ << ">";
        flag = true;
      }
      ofs_ << std::endl;
    }
  }
  ofs_.close();
  return ;
}