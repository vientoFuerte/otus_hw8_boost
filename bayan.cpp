#include "bayan.h"
#include <iostream>



void parse_command_line(BayanConfig& conf)
{
    // Выводим все директории из вектора
    std::cout << "Directories to scan:\n";
    for (const auto& dir : conf.directories) {
        std::cout << "  - " << dir << "\n";
    }

}

