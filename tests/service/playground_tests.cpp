#include <gtest/gtest.h>

#include "common.hpp"
#include "common/config.hpp"
#include "common/detail/project_utils.hpp"
#include "core/docker_wrapper.hpp"
#include "core/parser.hpp"
#include "core/service.hpp"

TEST(Playground, Run) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::PlaygroundTaskParams params;
    params.language = watchman::Language::PYTHON;
    params.project = watchman::parseProject(getFileContent(kPythonProject));
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_TRUE(response.output == "42\r\n");
    ASSERT_TRUE(!response.testsOutput.has_value());
}

TEST(Playground, Go) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::PlaygroundTaskParams params;
    params.language = watchman::Language::GO;
    params.project = watchman::parseProject(getFileContent(kGoProject));
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_EQ(response.output, "2 2");
}

TEST(Playground, DISABLED_Haskell_HelloWorld) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::PlaygroundTaskParams params;
    params.language = watchman::Language::HASKELL;
    params.project = watchman::parseProject(getFileContent(kHaskellProject));
    auto response = service.runPlayground(params);
    ASSERT_EQ(response.output, "Hello world!\r\n");
}

TEST(Playground, Rust_success) {
    watchman::Service service(watchman::readConfig(kParams.config));

    watchman::PlaygroundTaskParams params;
    params.language = watchman::Language::RUST;
    params.project = watchman::parseProject(getFileContent(kRustProject));
    auto response = service.runPlayground(params);
    ASSERT_EQ(response.output, "Hello world!\r\n");
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
}

TEST(Playground, C_plus_plus_success) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::PlaygroundTaskParams params;
    params.language = watchman::Language::CPP;
    params.project = watchman::parseProject(getFileContent(kCppProject));
    auto response = service.runPlayground(params);
    ASSERT_EQ(response.output, "42");
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
}

TEST(Playground, C_plus_plus_failure) {
    watchman::Service service(watchman::readConfig(kParams.config));

    watchman::PlaygroundTaskParams params;
    params.language = watchman::Language::CPP;
    params.project = watchman::parseProject(getFileContent(kCppProjectCompileError));
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == watchman::kUserCodeError);
}

TEST(Playground, TarDir) {
    watchman::detail::Directory rootDirectory =
        watchman::detail::jsonToDirectory(getFileContent(kFilesStructureAssets));
    auto const pathsContents = getPathsToFiles(rootDirectory);

    watchman::DockerWrapper dockerWrapper;
    watchman::RunContainer params{.image = "senjun_python", .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const pathInContainer = "/home/code_runner";

    bool const success = dockerWrapper.putArchive(
        {id, pathInContainer,
         watchman::detail::makeProjectTar({rootDirectory.name, pathsContents})});
    ASSERT_TRUE(success);
}
