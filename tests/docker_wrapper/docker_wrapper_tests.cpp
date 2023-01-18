#include <gtest/gtest.h>

#include "core/docker_wrapper.hpp"

using namespace watchman;

std::string const kPythonImage = "senjun_courses_python";

TEST(DockerWrapper, run_kill_delete) {
    for (size_t index = 0; index < 10; ++index) {
        DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
        DockerWrapper dockerWrapper;
        std::string const id = dockerWrapper.run(std::move(params));
        ASSERT_TRUE(!id.empty());
        ASSERT_TRUE(dockerWrapper.killContainer(id));
        ASSERT_TRUE(dockerWrapper.removeContainer(id));
    }
}

TEST(DockerWrapper, exec) {
    DockerWrapper dockerWrapper;
    //    DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
    //    std::string const id = dockerWrapper.run(std::move(params));
    //
    //    std::vector<std::string> args{"sh", "run.sh", "m.py"};

    std::string id = "8770ff79498e";
    std::vector<std::string> args{"touch", "m.py"};
    auto result = dockerWrapper.exec({args, id});
    args = {"echo", "m.py"};  // find combination for set text
    result = dockerWrapper.exec({std::move(args), std::move(id)});
    ASSERT_TRUE(result.exitCode == 0);
}
