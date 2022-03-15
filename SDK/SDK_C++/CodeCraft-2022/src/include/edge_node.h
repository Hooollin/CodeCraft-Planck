#include <string>

class EdgeNode {
 public:
  EdgeNode(std::string name, int bandwidth) : name_(name), bandwidth_(bandwidth) {}

  std::string GetName(){
    return name_;
  }
 private:
  std::string name_;
  int bandwidth_;
};