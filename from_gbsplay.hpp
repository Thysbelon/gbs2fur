#pragma once

#include <cstdint>
#include <string>
#include <cstdio>
#include <vector>
#include <cmath>
#include <set>
#include <array>
#include <tuple>
#include <cstring>
#include <variant>
#include <algorithm> // std::find
#include <map>

#include "gb_chip_state.hpp"

bool gbsplayStdout2songData(std::vector<gb_chip_state>& songData, std::string gbsFileName, int subsongNum, const uint32_t QUANTIZER, int timeInSeconds = 150);