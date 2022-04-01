
#include "edge_node.h"
#include <sstream>

std::string EdgeNode::ToString() {
  std::ostringstream oss;
  oss << "{"
      << "EdgeNode: " << name_ << ", bandwitdh: " << bandwidth_ << ","
      << std::endl;
  oss << "Serving ClientNodes:" << std::endl << "[";

  int count = 0;
  for (auto &p : serving_clientnode) {
    if (p != *serving_clientnode.begin()) oss << ", ";
    if (count > 0 && count % 20 == 0) oss << std::endl;
    oss << p;
    ++count;
  }
  oss << "]" << std::endl;
  oss << "}" << std::endl;
  return oss.str();
}
