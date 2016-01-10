#include "logging.hpp"

INITIALIZE_EASYLOGGINGPP

namespace mls {

void initLogging(int argc, char** argv, const FilePath& configFile) {
    START_EASYLOGGINGPP(argc, argv);

    if (!configFile.empty() && exists(configFile)) {
        el::Configurations conf(configFile.str());
        el::Loggers::setDefaultConfigurations(conf, true);
    }
}

}