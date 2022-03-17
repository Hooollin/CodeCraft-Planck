#pragma once

#include "strategy.h"
#include <vector>
#include <map>
#include <iostream>


class AverageStrategy: public Strategy {
public:
    AverageStrategy(
        std::string config = "/data/config.ini",
        std::string demand = "/data/demand.csv",
        std::string qos = "/data/qos.csv",
        std::string site_bandwidth = "/data/site_bandwidth.csv",
        std::string outputfile = "/output/solution.txt"): Strategy(config, demand, qos, site_bandwidth, outputfile) {
        }
    void HandleAllTimes() {
        
        for(int T = 0; T < GetInputParser()->GetT(); T ++) {
            GetInputParser()->ResetEdgeNode();
            HandleOneTimes(T);
        }
    }
    
    void CheckResult() {
        for(int T = 0; T < GetInputParser()->GetT(); T ++) {
            for(auto custome: GetInputParser()->GetClientNameList()) {
                int alloc_band = 0;
                for(auto site: GetInputParser()->GetEdgeNameList()) {
                    alloc_band += GetOutputParser()->GetAllocedBand(T, custome, site);
                }
                std::cout << T << ": " << "<" << custome << ", " << alloc_band << ">, ";
            }
            std::cout << std::endl;
        }
        
    }

private:
    std::vector<std::string> GetCustomRank();//得到用户的处理顺序
    std::vector<std::string> GetSiteRank();//得到处理节点的顺序,被使用最大限制贷款次数最少且服务用户最多
    void HandleOneSite(const std::string& site, int T);
    void HandleOneCustome(const std::string& custome, int T);
    void HandleOneCustomeAndCustom(EdgeNode* site, ClientNode* custome, int T, int demand);
    void HandleOneTimes(int T); //处理T时刻
};
