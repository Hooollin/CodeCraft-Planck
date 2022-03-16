#pragma once
#include<iostream>
#include <vector>
#include <map>

class AllocResourceT {
public:
    AllocResourceT(const std::vector<std::string>&, const std::vector<std::string>&);
    ~AllocResourceT();
    void Alloc(std::string custom, std::string site, int band) {
        custom_site_band[custom][site] = band;
    }
    int GetBand(const std::string custome, const std::string site) {
        return custom_site_band[custome][site];
    }
    void MakeOutput();
private:
    std::map<std::string, std::map<std::string, int>> custom_site_band;//每个时刻的分配矩阵，custom_site_band[custom][site], site 分配给custom的带宽
    std::vector<std::string> customnames, sitenames;
};

class OutputParser
{
public:
    OutputParser(int T, const std::vector<std::string>&, const std::vector<std::string>&);
    ~OutputParser() {}
    void AllocT(int T, std::string custom, std::string site, int band){
        custom_site_bands[T]->Alloc(custom, site, band);
    } //T时刻， 将site将band带宽分配给custom
    int GetAllocedBand(int T, std::string custome, std::string site) {
        return custom_site_bands[T]->GetBand(custome, site);
    }
    void MakeOutput();

private:
    std::vector<AllocResourceT*> custom_site_bands;
};

