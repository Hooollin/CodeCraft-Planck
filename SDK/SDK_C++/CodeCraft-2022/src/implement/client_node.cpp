
#include "client_node.h"
#include <sstream>

std::string ClientNode::ToString() {
  std::ostringstream oss;
  oss << "{"
      << "ClientNode: " << name_ << ", "
      << "demands: " << demand_.size() << std::endl;
  oss << " [";
  for (int i = 0; i < demand_.size(); ++i) {
    if (i != 0) oss << ", ";
    if (i > 0 && i % 20 == 0) oss << std::endl;
    oss << demand_[i];
  }
  oss << "]," << std::endl;
  oss << "available EdgeNodes:" << std::endl;
  oss << "[";
  int count = 0;
  for (auto &p : available_edgenode) {
    if (p != *available_edgenode.begin()) oss << ", ";
    if (count > 0 && count % 20 == 0) oss << std::endl;
    oss << p;
    ++count;
  }
  oss << "]," << std::endl;
  oss << "}" << std::endl;
  return oss.str();
}
