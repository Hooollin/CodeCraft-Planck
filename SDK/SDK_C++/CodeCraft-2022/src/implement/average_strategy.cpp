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

void AverageStrategy::AdvisorProcess(int T) {
  auto customnames = GetInputParser()->GetClientNameList();
  auto sitenames = GetInputParser()->GetEdgeNameList();
  for (const std::string& cust: customnames) {
    for (const std::string& site: sitenames) {
      int prealloc = GetAdvisor()->Predict(T, site, cust);
      GetOutputParser()->AllocT(T, cust, site, prealloc);
      GetInputParser()->GetClientNodeMap()[cust]->DecDemand(prealloc, T);
      GetInputParser()->GetEdgeNodeMap()[site]->DecRemain(prealloc);
    }
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