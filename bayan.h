#pragma once

#include <vector>
#include <string>
#include <iostream>


struct BayanConfig {
    std::vector<std::string> directories;
    std::vector<std::string> exclude_dirs;
    int scan_depth = -1;
    size_t min_size = 1;
    
};

void bayan_parser(BayanConfig& conf);
