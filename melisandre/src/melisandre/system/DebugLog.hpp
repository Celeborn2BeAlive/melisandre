#pragma once

#include <fstream>

namespace mls {

void startDebugLog(const char* file, int line);

std::ostream& debugLog();

}

#define BNZ_START_DEBUG_LOG startDebugLog(__FILE__, __LINE__)
