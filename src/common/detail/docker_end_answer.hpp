#pragma once

#include "common/common.hpp"

#include <string>

namespace watchman {

Response getCourseResponse(std::string const & message);
Response getPlaygroungResponse(std::string const & message);
Response getPracticeResponse(std::string const & message);

}  // namespace watchman
