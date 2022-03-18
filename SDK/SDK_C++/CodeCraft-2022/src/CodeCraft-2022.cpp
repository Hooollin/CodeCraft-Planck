#include "test.h"
#include "distributor.h"

int main() {
//  Test test_example(3);
//  test_example.TestPreDeal();

  Distributor distributor(1);
  distributor.DayDistribute();
  return 0;
}
