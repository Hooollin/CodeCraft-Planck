#include <iostream>
#include "input_parser.h"
#include "strategy.h"
#include "average_strategy.h"

static bool local = 1;

int main() {
  Strategy *st;
  if (local) {
    st = new AverageStrategy("../data/config.ini", "../data/demand.csv",
                             "../data/qos.csv", "../data/site_bandwidth.csv", "../data/solution.txt");
  } else {
    st = new AverageStrategy();
  }
  st->HandleAllTimes();
  st->MakeOutput(local);
  st->CheckResult();
  return 0;
}
