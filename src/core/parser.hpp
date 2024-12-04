#pragma once

#include "common/run_params.hpp"
#include "core/code_launcher/response.hpp"

#include <string>

namespace watchman {

// parse request
CourseTaskParams parseTask(std::string const & body);
PlaygroundTaskParams parsePlayground(std::string const & body);
Project parseProject(std::string const & json);

RunPracticeParams parsePractice(std::string const & body);

// create json responses
std::string makeJsonCourse(Response && response);
std::string makeJsonPlayground(Response && response);

std::string makeErrorJson(std::string const & message);

}  // namespace watchman
