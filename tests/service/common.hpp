#pragma once

#include <string>

struct ServiceParams {
    std::string const config = std::string{TEST_DATA_DIR} + "config.json";
};

ServiceParams const kParams;
