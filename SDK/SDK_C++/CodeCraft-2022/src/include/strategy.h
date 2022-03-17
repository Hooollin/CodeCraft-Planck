#pragma once
#include "client_node.h"
#include "edge_node.h"
#include "input_parser.h"
#include "output_parser.h"

#include <iostream>
#include <map>
#include <vector>
class Strategy {
 public:
  Strategy(InputParser *input_parser, OutputParser *output_parser)
      : input_parser_(input_parser), output_parser_(output_parser) {}
  InputParser *GetInputParser() { return input_parser_; }
  OutputParser *GetOutputParser() { return output_parser_; }

  virtual void HandleAllTimes() = 0;
  void MakeOutput(bool local = true) {
    if (local)
      output_parser_->LocalDisplay();
    else
      output_parser_->MakeOutput();
  }

 protected:
  InputParser *input_parser_;
  OutputParser *output_parser_;
};