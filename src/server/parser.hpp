#pragma once

#include "common/common.hpp"

#include <string>

namespace watchman {

class Parser {
public:
    RunTaskParams parse(std::string const & body);
};
}  // namespace watchman
