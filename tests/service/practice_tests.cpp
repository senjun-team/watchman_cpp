#include <gtest/gtest.h>

#include "common.hpp"
#include "core/docker_wrapper.hpp"
#include "core/parser.hpp"
#include "core/service.hpp"

TEST(Practice, RunTest) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::RunPracticeParams params = watchman::parsePractice(getJson(kPythonPracticeRun));
    auto response = service.runPractice(params);
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);

    params = watchman::parsePractice(getJson((kPythonPracticeTest)));
    response = service.runPractice(params);
    ASSERT_TRUE(response.sourceCode == watchman::kTestsError);
}
