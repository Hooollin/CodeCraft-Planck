#include "average_strategy.h"
#include <algorithm>

std::vector<std::string> AverageStrategy::GetCustomRank() {
  auto customnames = GetInputParser()->GetClientNameList();

  auto cmp = [&](std::string &cust1, std::string &cust2) {
    auto &clientmap = GetInputParser()->GetClientNodeMap();
    return clientmap[cust1]->GetAvailableEdgeNodeCount() >
           clientmap[cust2]->GetAvailableEdgeNodeCount();
  };
  std::sort(customnames.begin(), customnames.end(), cmp);
  return customnames;
}

std::vector<std::string> AverageStrategy::GetSiteRank() {
  std::vector<std::string> sitenames = GetInputParser()->GetEdgeNameList();
  auto cmp = [&](std::string &site1, std::string &site2) {
    auto &edgemap = GetInputParser()->GetEdgeNodeMap();
    if (edgemap[site1]->GetLimitCnt() == edgemap[site2]->GetLimitCnt())
      return edgemap[site1]->GetServingClientNodeCount() >
             edgemap[site2]->GetServingClientNodeCount();
    return edgemap[site1]->GetLimitCnt() < edgemap[site2]->GetLimitCnt();
  };
  std::sort(sitenames.begin(), sitenames.end(), cmp);
  return sitenames;
}

void AverageStrategy::HandleOneSite(const std::string &sitename, int T) {
  EdgeNode *site = GetInputParser()->GetEdgeNodeMap()[sitename];
  std::vector<ClientNode *> customes = site->GetServingClientNode();
  auto cmp = [&](ClientNode *c1, ClientNode *c2) {
    return c1->GetAvailableEdgeNodeCount() < c2->GetAvailableEdgeNodeCount();
  };
  std::sort(customes.begin(), customes.end(), cmp);
  for (auto cust : customes) {
    HandleOneCustome(site, cust, T);
    site->IncLimitCnt();
  }
}

void AverageStrategy::HandleOneCustome(EdgeNode *site, ClientNode *custome,
                                       int T) {
  int demand = custome->GetDemand(T);
  auto outputParser = GetOutputParser();
  if (site->GetRemain() < demand) {
    demand = site->GetRemain();
    assert(demand >= 0);
  }
  site->DecRemain(demand);
  custome->DecDemand(demand, T);
  outputParser->AllocT(T, custome->GetName(), site->GetName(), demand);
}

void AverageStrategy::HandleOneTimes(int T) {
  auto sites = GetSiteRank();
  for (int i = 0; i < sites.size(); i++) {
    HandleOneSite(sites[i], T);
  }
}