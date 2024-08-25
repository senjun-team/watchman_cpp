#include <gtest/gtest.h>

#include "common.hpp"
#include "core/docker_wrapper.hpp"
#include "core/parser.hpp"
#include "core/service.hpp"

#include <fstream>

namespace {

std::string getJson(std::string const & path) {
    std::ifstream file(getAssetPath(path));
    std::stringstream json;
    json << file.rdbuf();
    return json.str();
}

}  // namespace

TEST(Playground, Run) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "python_playground";

    watchman::RunProjectParams params;
    params.containerType = std::move(containerType);
    params.project = watchman::parseProject(getJson(kPythonProject));
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_TRUE(response.output == "42\r\n");
    ASSERT_TRUE(!response.testsOutput.has_value());
}

TEST(Playground, Go) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "golang_playground";

    watchman::RunProjectParams params;
    params.containerType = std::move(containerType);
    params.project = watchman::parseProject(getJson(kGoProject));
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_EQ(response.output, "2 2");
}

TEST(Playground, DISABLED_Haskell_HelloWorld) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "haskell_playground";

    watchman::RunProjectParams params;
    params.containerType = std::move(containerType);
    params.project = watchman::parseProject(getJson(kHaskellProject));
    auto response = service.runPlayground(params);
    ASSERT_EQ(response.output, "Hello world!\r\n");
}


TEST(Playground, Rust_success) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "rust_playground";

    watchman::RunProjectParams params;
    params.containerType = std::move(containerType);
    params.project = watchman::parseProject(getJson(kRustProject));
    auto response = service.runPlayground(params);
    ASSERT_EQ(response.output, "Hello world!\r\n");
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
}

TEST(Playground, C_plus_plus_success) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "cpp_playground";

    watchman::RunProjectParams params;
    params.containerType = std::move(containerType);
    params.project = watchman::parseProject(getJson(kCppProject));
    auto response = service.runPlayground(params);
    ASSERT_EQ(response.output, "42");
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
}

TEST(Playground, C_plus_plus_failure) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "cpp_playground";

    watchman::RunProjectParams params;
    params.containerType = std::move(containerType);
    params.project = watchman::parseProject(getJson(kCppProjectCompileError));
    auto response = service.runPlayground(params);
    ASSERT_TRUE(response.sourceCode == watchman::kUserCodeError);
}

TEST(Playground, TarDir) {
    watchman::Directory rootDirectory = watchman::jsonToDirectory(getJson(kFilesStructureAssets));
    auto const pathsContents = getPathsToFiles(rootDirectory);

    watchman::DockerWrapper dockerWrapper;
    watchman::RunContainer params{.image = "senjun_courses_python", .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const pathInContainer = "/home/code_runner";

    bool const success = dockerWrapper.putArchive(
        {id, pathInContainer, watchman::makeProjectTar({rootDirectory.name, pathsContents}).str()});
    ASSERT_TRUE(success);
}
