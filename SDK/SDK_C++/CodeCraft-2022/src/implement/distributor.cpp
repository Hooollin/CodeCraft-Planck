
#include "distributor.h"

void Distributor::DayDistribute() {
  //定义并处理处理输入
  InputParser input_parser(model_, &data_);
  input_parser.Parse();
  //定义输出类
  OutputParser output_parser(model_, &data_);
  //预处理
  LHLPreDistribution pre_distribution(&data_);
  pre_distribution.Distribute();
  int allday = data_.GetAllDays();
  std::vector<int> days_order = data_.GetDaysOrder();
  //进行每日处理
  for (int i = 0; i < allday; i++) {
    int nowaday = days_order[i];
    LHKStrategy strategy(nowaday, &data_);
    strategy.Distribute();
  }
  output_parser.StandradOutput();
  return;
}