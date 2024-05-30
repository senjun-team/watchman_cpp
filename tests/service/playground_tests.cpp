#include <gtest/gtest.h>

#include "common.hpp"
#include "core/service.hpp"

TEST(Playground, Run) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "python_playground";
    std::string sourceCode = "print(42)";

    watchman::RunCodeParams params{std::move(containerType), std::move(sourceCode), {}};
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_TRUE(!response.output.empty());
    ASSERT_TRUE(!response.testsOutput.has_value()); // todo fix in code this behaviour
}