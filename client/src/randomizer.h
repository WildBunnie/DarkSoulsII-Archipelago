#pragma once

#include <map>
#include <set>
#include <vector>
#include <cstdint>
#include <string>
#include "archipelago.h"

enum ClassRandomizationFlag {
    ONE_HANDABLE = 0,
    TWO_HANDABLE = 1,
    FULL_RANDOM = 2
};

void override_item_params(APState& state);
void randomize_starter_classes(std::string seed_str, ClassRandomizationFlag flag);