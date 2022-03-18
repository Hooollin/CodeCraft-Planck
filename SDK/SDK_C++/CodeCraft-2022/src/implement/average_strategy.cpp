#include "average_strategy.h"
#include <algorithm>

std::vector<std::string> AverageStrategy::GetCustomRank() {
  auto customnames = GetInputParser()->GetClientNameList();

  auto cmp = [&](std::string &cust1, std::string &cust2) {
    auto &clientmap = GetInputParser()->GetClientNodeMap();
    return clientmap[cust1]->GetAvailableEdgeNodeCount() <
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
    // HandleOneCustomeAndCustom(site, cust, T);
    site->IncLimitCnt();
  }
}

void AverageStrategy::HandleOneCustomeAndCustom(EdgeNode *site,
                                                ClientNode *custome, int T,
                                                int demand) {
  auto outputParser = GetOutputParser();
  site->DecRemain(demand);
  custome->DecDemand(demand, T);
  outputParser->AllocT(T, custome->GetName(), site->GetName(), demand);
}

void AverageStrategy::HandleOneCustome(const std::string &custome, int T) {
  ClientNode *client = GetInputParser()->GetClientNodeMap()[custome];
  std::vector<EdgeNode *> sites = client->GetEdgeNodeList();
  auto cmp = [&](EdgeNode *site1, EdgeNode *site2) {
    if (site1->GetRemain() == site2->GetRemain())
      return site1->GetLimitCnt() < site2->GetLimitCnt();
    return site1->GetRemain() > site2->GetRemain();
  };
  sort(sites.begin(), sites.end(), cmp);
  int used_cnt = 0;
  for (auto site : sites) {
    if (client->GetDemand(T) == 0) break;
    int demand = client->GetDemand(T);
    demand = std::min(site->GetRemain(), demand);
    if (site->GetLimitCnt() >= GetInputParser()->GetT() * 0.05) {
      demand =
          (demand + sites.size() - used_cnt - 1) / (sites.size() - used_cnt);
    }
    HandleOneCustomeAndCustom(site, client, T, demand);
    assert(site->GetRemain() >= 0);
    used_cnt += 1;
  }
  assert(client->GetDemand(T) == 0);
}

void AverageStrategy::HandleOneTimes(int T) {
  // auto sites = GetSiteRank();
  auto customes = GetCustomRank();
  for (int i = 0; i < customes.size(); i++) {
    HandleOneCustome(customes[i], T);
    for (auto site : GetInputParser()->GetEdgeNameList()) {
      EdgeNode *edge = GetInputParser()->GetEdgeNodeMap()[site];
      if (edge->GetRemain() == 0) edge->IncLimitCnt();
    }
  }
}