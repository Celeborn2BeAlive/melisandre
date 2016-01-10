#pragma once

#include <string>

#include <easyloggingpp/easylogging++.h>
#include "files.hpp"

namespace mls {

void initLogging(int argc, char** argv, const FilePath& configFile = FilePath{ "" });

using el::Logger;

template<typename T>
inline Logger* getLogger(T&& name) {
    return el::Loggers::getLogger(std::forward<T>(name));
}

}