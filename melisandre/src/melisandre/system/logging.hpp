#pragma once

#include <string>

#include <easyloggingpp/easylogging++.h>
#include "files.hpp"

namespace mls {

void initLogging(int argc, char** argv, const FilePath& configFile = FilePath{ "" });

using el::Logger;

template<typename String>
inline Logger* getLogger(String&& name) {
    return el::Loggers::getLogger(std::forward<String>(name));
}

}