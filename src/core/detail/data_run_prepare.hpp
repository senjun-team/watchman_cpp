#pragma once
#include "common/run_params.hpp"

namespace watchman::detail {

std::string prepareData(RunTaskParams const & taskParams);
std::string prepareData(RunProjectParams const & taskParams);
std::string prepareData(RunPracticeParams const & taskParams);

}  // namespace watchman::detail
