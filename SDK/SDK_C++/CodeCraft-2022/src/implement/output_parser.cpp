#include "output_parser.h"

AllocResourceT::AllocResourceT(const std::vector<std::string>& customnames, const std::vector<std::string>& sitenames, const std::string& filename) {
    for(const auto& custome: customnames) {
        for(const auto& site: sitenames) {
            this->custom_site_band[custome][site] = 0;
        }
    }
    this->customnames = customnames;
    this->sitenames = sitenames;
    this->filename = filename;
}

OutputParser::OutputParser(int T, const std::vector<std::string>& customnames, const std::vector<std::string>& sitenames, const std::string& filename) {
    for(int i = 0; i < T; i ++){
        AllocResourceT *art = new AllocResourceT(customnames, sitenames, filename);
        custom_site_bands.push_back(art);
    }
}

void AllocResourceT::MakeOutput(bool tail) {
    ofs.open(filename, std::ios::app | std::ios::out);
    for(int i = 0; i < customnames.size(); i ++) {
        std::string custom = customnames[i];
        ofs << custom << ":";
        std::vector<std::string> alloced;
        for(int i = 0; i < sitenames.size(); i ++)
            if(custom_site_band[custom][sitenames[i]] > 0)
                alloced.push_back(sitenames[i]);
        for(int i = 0; i < alloced.size(); i ++) {
            ofs << "<" << alloced[i] << "," << custom_site_band[custom][alloced[i]] << ">";
            if(i < alloced.size()-1) ofs << ",";
        }
        if (i < customnames.size() - 1)
            ofs << std::endl;
        if (i == customnames.size() - 1 && tail)
            ofs << std::endl;
    }
    ofs.close();
}

void AllocResourceT::LocalDisplay() {
    for(auto& custom: customnames) {
        std::cout << custom << ":";
        std::vector<std::string> alloced;
        for(int i = 0; i < sitenames.size(); i ++)
            if(custom_site_band[custom][sitenames[i]] > 0)
                alloced.push_back(sitenames[i]);
        for(int i = 0; i < alloced.size(); i ++) {
            std::cout << "<" << alloced[i] << "," << custom_site_band[custom][alloced[i]] << ">";
            if(i < alloced.size()-1) std::cout << ",";
        }
        std::cout << std::endl;
    }
}

void OutputParser::LocalDisplay() {
    for(auto& art: custom_site_bands){
        art->LocalDisplay();
        std::cout << std::endl;
    }
}

void OutputParser::MakeOutput() {
    bool tail = true;
    for(int i = 0; i < custom_site_bands.size(); i ++){
        if(i == custom_site_bands.size() - 1) tail = false;
        custom_site_bands[i]->MakeOutput(tail);
    }
}