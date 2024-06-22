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
    ASSERT_TRUE(response.output == "42");
    ASSERT_TRUE(!response.testsOutput.has_value());
}

TEST(Playground, Go) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "golang_playground";
    std::string sourceCode =
        "package main\nimport \"fmt\"\nfunc main() {\nx := 2\nfmt.Printf(\"%d %v\", x, x)\n}";

    watchman::RunCodeParams params{std::move(containerType), std::move(sourceCode), {}};
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_EQ(response.output, "2 2");
}

TEST(Playground, Haskell) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "haskell_playground";
    std::string sourceCode = "module Main where\nmain :: IO ()\nmain = putStrLn (show 129";

    watchman::RunCodeParams params{std::move(containerType), std::move(sourceCode), {}};
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == 1);
}
