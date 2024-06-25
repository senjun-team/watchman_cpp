#pragma once

#include "common/common.hpp"

#include <string>

namespace watchman {

// parse request
RunTaskParams parse(std::string const & body, Api api);

// create json responses
std::string makeJsonCourse(Response && response);
std::string makeJsonPlayground(Response && response);

}  // namespace watchman
