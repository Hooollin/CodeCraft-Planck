#pragma once
#include "edge_node.h"
#include "client_node.h"
#include "input_parser.h"
#include "output_parser.h"

#include <vector>
#include <map>
#include <iostream>
class Strategy {
public:
    Strategy(std::string config = "/data/config.ini",
            std::string demand = "/data/demand.csv",
            std::string qos = "/data/qos.csv",
            std::string site_bandwidth = "/data/site_bandwidth.csv") {
                        inputParser = new InputParser(config, demand, qos, site_bandwidth);
                        outputParser = new OutputParser(inputParser->GetT(), inputParser->GetClientNameList(), inputParser->GetEdgeNameList());
                    }
    ~Strategy() {
        delete inputParser;
        delete outputParser;
        inputParser = nullptr;
        outputParser = nullptr;
    }
    InputParser* GetInputParser() {return inputParser;}
    OutputParser* GetOutputParser() {return outputParser;}

    virtual void HandleAllTimes();
    void MakeOutput() {
        outputParser->MakeOutput();
    }
private:
    InputParser* inputParser;
    OutputParser* outputParser;
};