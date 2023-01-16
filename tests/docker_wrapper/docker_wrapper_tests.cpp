#include <gtest/gtest.h>

#include "core/docker_wrapper.hpp"

using namespace watchman;

TEST(DockerWrapper, run_kill_delete) {
    for (size_t index = 0; index < 10; ++index) {
        std::string imageName = "senjun_courses_python";
        DockerRunParams params{.image = std::move(imageName), .tty = true, .memoryLimit = 7000000};
        DockerWrapper dockerWrapper;
        std::string const id = dockerWrapper.run(std::move(params));
        ASSERT_TRUE(!id.empty());
        ASSERT_TRUE(dockerWrapper.killContainer(id));
        ASSERT_TRUE(dockerWrapper.removeContainer(id));
    }
}

TEST(DockerWrapper, exec) {
    DockerWrapper dockerWrapper;

    for (size_t index = 0; index < 10; ++index) {
        std::string id = "c67c6d15ce9b";
        std::vector<std::string> args{"sh", "run.sh", "m.py"};
        auto result = dockerWrapper.exec({std::move(args), std::move(id)});
        ASSERT_TRUE(result.exitCode == 0);
    }
}