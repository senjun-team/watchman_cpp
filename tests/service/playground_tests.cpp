#include <gtest/gtest.h>

#include "common.hpp"
#include "core/docker_wrapper.hpp"
#include "core/parser.hpp"
#include "core/service.hpp"

#include <fstream>

TEST(Playground, Run) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "python_playground";
    std::string sourceCode = "print(42)";

    watchman::RunCodeParams params{std::move(containerType), std::move(sourceCode), {}};
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_TRUE(response.output == "42\r\n");
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

TEST(Playground, C_plus_plus_success) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "cpp_playground";
    std::string sourceCode =
        "#include <iostream>\nusing namespace std;\n int main(){\n\tcout<<\"Hello, world\";}";

    watchman::RunCodeParams params{std::move(containerType), std::move(sourceCode), {}};
    auto response = service.runPlayground(params);
    ASSERT_EQ(response.output, "Hello, world");
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
}

TEST(Playground, C_plus_plus_failure) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "cpp_playground";
    auto sourceCode = "lalalala;";

    watchman::RunCodeParams params{std::move(containerType), std::move(sourceCode), {}};
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == watchman::kUserCodeError);
}

TEST(Playground, DISABLED_TarDir) {
    std::ifstream file(getAssetPath(kFilesStructureAssets));
    std::stringstream json;
    json << file.rdbuf();

    watchman::Directory rootDirectory = watchman::jsonToDirectory(json.str());
    auto const pathsContents = getPathsToFiles(rootDirectory);

    watchman::DockerWrapper dockerWrapper;
    watchman::RunContainer params{.image = "senjun_courses_python", .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const pathInContainer = "/home/code_runner";

    bool const success = dockerWrapper.putArchive(
        {id, pathInContainer, watchman::makeProjectTar({rootDirectory.name, pathsContents}).str()});
    ASSERT_TRUE(success);
}
