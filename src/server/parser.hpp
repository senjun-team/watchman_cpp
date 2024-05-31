#pragma once

#include "common/common.hpp"

#include <string>

namespace watchman {

RunTaskParams parse(std::string const & body, Api api);
}  // namespace watchman
