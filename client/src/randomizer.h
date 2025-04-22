#pragma once

#include <map>
#include <set>
#include <vector>
#include <cstdint>
#include <string>

enum ClassRandomizationFlag {
    ONE_HANDABLE = 0,
    TWO_HANDABLE = 1,
    FULL_RANDOM = 2
};

void override_item_params(std::map<int32_t, int32_t> rewards, std::string seed, std::set<int32_t> ignore);
void randomize_starter_classes(std::string seed_str, ClassRandomizationFlag flag);