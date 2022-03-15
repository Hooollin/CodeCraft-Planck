#include <fstream>
#include <map>
#include <string>
#include <vector>

class EdgeNode;
class UserNode;

class InputParser {
 public:
  InputParser(std::string config = "/data/config.ini",
              std::string demand = "/data/demand.csv",
              std::string qos = "/data/qos.csv",
              std::string site_bandwidth = "/data/site_bandwidth.csv")
      : config_(config),
        demand_(demand),
        qos_(qos),
        site_bandwidth_(site_bandwidth) {}

  InputParser(InputParser &) = delete;
  InputParser(InputParser &&) = delete;

  std::map<std::string, EdgeNode> GetEdgeNodeMap();

  std::map<std::string, UserNode> GetUserNodeMap();

  void Parse();

 private:
  void ReadConfigFile();

  void ParseDemandFile();

  void ParseQosFile();

  void ParseSiteBandWidthFile();

  void SplitString(std::string &, char, std::vector<std::string> &);

  std::string demand_;
  std::string config_;
  std::string qos_;
  std::string site_bandwidth_;

  std::map<std::string, EdgeNode> edgenode_;
  std::map<std::string, UserNode> usernode_;

  std::ifstream ifs_;
};