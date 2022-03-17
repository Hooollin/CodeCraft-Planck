#include <iostream>
#include "average_strategy.h"
#include "input_parser.h"
#include "strategy.h"

static bool local = true;

int main() {
  InputParser *input_parser;
  OutputParser *output_parser;
  Strategy *st;
  if (local) {
    input_parser =
        new InputParser("../data/config.ini", "../data/demand.csv",
                        "../data/qos.csv", "../data/site_bandwidth.csv");
        input_parser->Init();

    output_parser = new OutputParser(
        input_parser->GetT(), input_parser->GetClientNameList(),
        input_parser->GetEdgeNameList(), "../output/solution.txt");
  } else {
    input_parser = new InputParser();
    input_parser->Init();

    output_parser = new OutputParser(
        input_parser->GetT(), input_parser->GetClientNameList(),
        input_parser->GetEdgeNameList(), "output/solution.txt");
  }

  st = new AverageStrategy(input_parser, output_parser);
  st->HandleAllTimes();
  st->MakeOutput();
  return 0;
}
