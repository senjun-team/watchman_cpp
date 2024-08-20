#pragma once

#include "common/common.hpp"

#include <string>

namespace watchman {

// parse request
RunTaskParams parseTask(std::string const & body);
RunProjectParams parsePlayground(std::string const & body);
Project parseProject(std::string const & json);

// create json responses
std::string makeJsonCourse(Response && response);
std::string makeJsonPlayground(Response && response);

}  // namespace watchman
