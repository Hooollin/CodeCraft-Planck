#include "test.h"
#include "distributor.h"

int main() {
//  Test test_example(3);
//  test_example.TestEdgeDistribution();

  Distributor distributor(3);
  distributor.DayDistribute();
  return 0;
}
