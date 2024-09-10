#pragma once

#include "common/common.hpp"

#include <string>

namespace watchman {

// parse request
RunTaskParams parseTask(std::string const & body);

RunProjectParams parsePlayground(std::string const & body);
Project parseProject(std::string const & json);

RunPracticeParams parsePractice(std::string const & body);

// create json responses
std::string makeJsonCourse(Response && response);
std::string makeJsonPlayground(Response && response);

std::string makeErrorJson(std::string const & message);

}  // namespace watchman
