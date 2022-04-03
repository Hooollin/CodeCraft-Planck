#pragma once
#include "data.h"

class DayDistribution {
 public:
  DayDistribution(int &day, Data *data) : days_(day), data_(data) {}

  virtual void Distribute() {}

 protected:
  //今天是第几天
  int days_;
  //数据
  Data *data_;
};
