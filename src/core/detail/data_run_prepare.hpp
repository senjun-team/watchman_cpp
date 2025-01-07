#pragma once
#include "common/run_params.hpp"

namespace watchman::detail {

std::string prepareData(ChapterTaskParams const & taskParams);
std::string prepareData(PlaygroundTaskParams const & taskParams);
std::string prepareData(RunPracticeParams const & taskParams);

}  // namespace watchman::detail
