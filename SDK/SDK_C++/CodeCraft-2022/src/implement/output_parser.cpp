#include "output_parser.h"
#include <algorithm>

AllocResourceT::AllocResourceT(const std::vector<std::string> &customnames,
                               const std::vector<std::string> &sitenames,
                               const std::string &filename) {
  for (const auto &custome : customnames) {
    for (const auto &site : sitenames) {
      this->custom_site_band[custome][site] = 0;
    }
  }
  this->customnames = customnames;
  this->sitenames = sitenames;
  this->filename = filename;
}

OutputParser::OutputParser(int T, const std::vector<std::string> &customnames,
                           const std::vector<std::string> &sitenames,
                           const std::string &filename) {
  for (int i = 0; i < T; i++) {
    AllocResourceT *art = new AllocResourceT(customnames, sitenames, filename);
    custom_site_bands.push_back(art);
  }
}

void AllocResourceT::MakeOutput() {
  ofs.open(filename, std::ios::app | std::ios::out);
  for (int i = 0; i < customnames.size(); i++) {
    std::string custom = customnames[i];
    ofs << custom << ":";
    std::vector<std::string> alloced;
    for (int j = 0; j < sitenames.size(); j++)
      if (custom_site_band[custom][sitenames[j]] > 0)
        alloced.push_back(sitenames[j]);
    for (int j = 0; j < alloced.size(); j++) {
      ofs << "<" << alloced[j] << "," << custom_site_band[custom][alloced[j]]
          << ">";
      if (j < alloced.size() - 1) ofs << ",";
    }
    ofs << std::endl;
  }
  ofs.close();
}

void AllocResourceT::LocalDisplay() {
  for (auto &custom : customnames) {
    std::cout << custom << ":";
    std::vector<std::string> alloced;
    for (int i = 0; i < sitenames.size(); i++)
      if (custom_site_band[custom][sitenames[i]] > 0)
        alloced.push_back(sitenames[i]);
    for (int i = 0; i < alloced.size(); i++) {
      std::cout << "<" << alloced[i] << ","
                << custom_site_band[custom][alloced[i]] << ">";
      if (i < alloced.size() - 1) std::cout << ",";
    }
    std::cout << std::endl;
  }
}

void OutputParser::CalCost() {
  std::map<std::string, std::vector<int>> site_cost;
  for (auto &art : custom_site_bands) {
    for (std::string site : art->GetSiteNames()) {
      int cost = 0;
      if (!site_cost.count(site)) site_cost[site] = {};
      for (std::string cust : art->GetCustomNames()) {
        cost += art->GetBand(cust, site);
      }
      site_cost[site].emplace_back(cost);
    }
  }
  int total_cost = 0;
  int percent95_idx = int((site_cost.size() - 1) * 0.95);
  for (auto [site, costs] : site_cost) {
    std::sort(costs.begin(), costs.end());
    total_cost += costs[percent95_idx];
  }
  std::cout << "the total cost is: " << total_cost << std::endl;
}

void OutputParser::LocalDisplay() {
  for (auto &art : custom_site_bands) {
    art->LocalDisplay();
    std::cout << std::endl;
  }
  CalCost();
}

void OutputParser::MakeOutput() {
  for (int i = 0; i < custom_site_bands.size(); i++) {
    custom_site_bands[i]->MakeOutput();
  }
}