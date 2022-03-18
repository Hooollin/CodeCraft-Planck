#pragma once
#include "client_node.h"
#include "edge_node.h"
#include <vector>
#include <algorithm>

struct LoadingNode{
  long long loading;
  int which_day;
  EdgeNode *which_edge;
};
/*
  1. 计算出edge节点在所有时刻的loading，添加到数组all_loading中;
  2. 每轮对all_loading做make_heap的操作，取出最处的loading所属的edgenode处理；
  3. 如果edgenode用完了5%的次数，回到2;
  4. 优先处理edgenode中连接edge数量最少的client，并更新all_loading中对应元素
*/
class Advisor{
  public:
    Advisor(int T, std::vector<ClientNode*> &clientnodes, std::vector<EdgeNode*> &edgenodes): T_(T), clientnodes_(clientnodes), edgenodes_(edgenodes){}

    void Init();

    void MakeOverallSuggestion();

    int Predict(int day, std::string edge_name, std::string client_name);

  private:
    int T_;
    std::vector<ClientNode*> clientnodes_;
    std::vector<EdgeNode*> edgenodes_;

    std::map<std::string, int> max_loading_change_;
    std::map<std::string, std::map<int, std::vector<LoadingNode*>>> client_to_day_specified_loading_node_;
    std::vector<LoadingNode*> max_loadings_;

    std::map<int, std::map<std::string, std::map<std::string, int>>> hint_;
};
