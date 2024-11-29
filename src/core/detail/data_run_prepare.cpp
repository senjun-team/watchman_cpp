#include "core/detail/data_run_prepare.hpp"

#include "common/common.hpp"
#include "common/project.hpp"

namespace watchman::detail {

std::string prepareData(RunTaskParams const & runTaskParams) {
    std::vector<CodeFilename> data{{runTaskParams.sourceRun, kFilenameTask},
                                   {runTaskParams.sourceTest, kFilenameTaskTests}};
    return makeTar(std::move(data));
}

std::string prepareData(RunProjectParams const & taskParams) {
    return makeProjectTar(taskParams.project);
}

std::string prepareData(RunPracticeParams const & taskParams) {
    return makeProjectTar(taskParams.practice.project);
}

}  // namespace watchman::detail
