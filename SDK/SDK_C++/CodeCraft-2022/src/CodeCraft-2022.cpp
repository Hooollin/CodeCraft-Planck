#include "distributor.h"
#include "test.h"
int main() {
//  Test test(3);
//  test.TestAll();

  Distributor distributor(1);
  distributor.DayDistribute();
  return 0;
}
