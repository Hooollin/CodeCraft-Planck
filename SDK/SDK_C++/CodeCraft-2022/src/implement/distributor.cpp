
#include "distributor.h"

void Distributor::DayDistribute() {
  //定义并处理处理输入
  InputParser input_parser(model_,&data_);
  input_parser.Parse();
  //定义输出类
  OutputParser output_parser(model_,&data_);
  //预处理，获得预处理分配
  PreDistribution pre_distribution(&data_);
  pre_distribution.Distribute();
  int allday = data_.GetAllDays();

  //进行每日处理
  for (int i = 0; i < allday; i++) {
    ClientDayDistribution day_distribution(i, &data_);
    day_distribution.DistributeBalanced();
  }
  output_parser.StandradOutput();
  return;
}