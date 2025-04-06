#include <gtest/gtest.h>

#include "common.hpp"
#include "core/parser.hpp"
#include "core/service.hpp"

TEST(Practice, RunTest) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::RunPracticeParams params =
        watchman::parsePractice(getFileContent(kPythonPracticeRun));
    auto response = service.runPractice(params);
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);

    params = watchman::parsePractice(getFileContent((kPythonPracticeTest)));
    response = service.runPractice(params);
    ASSERT_TRUE(response.sourceCode == watchman::kTestsError);
}

TEST(Practice, RunCpp) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::RunPracticeParams params = watchman::parsePractice(getFileContent(kCppPracticeRun));
    auto response = service.runPractice(params);
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);

    params = watchman::parsePractice(getFileContent((kCppPracticeTest)));
    response = service.runPractice(params);
    ASSERT_TRUE(response.sourceCode == watchman::kTestsError);
}
