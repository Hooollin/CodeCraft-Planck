#pragma once

#include <math.h>
#include <algorithm>
#include <cassert>
#include "data.h"
class PreDistribution {
 public:
  //初始化
  PreDistribution(Data *data) : data_(data) {}

  virtual void Distribute() {}

 protected:
  Data *data_;
};