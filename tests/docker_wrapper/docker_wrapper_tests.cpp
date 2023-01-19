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
    DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));

    std::string const reference = "42";
    std::vector<std::string> const args{"echo", reference};

    auto result = dockerWrapper.exec({args, id});
    ASSERT_TRUE(result.exitCode == 0);
    ASSERT_EQ(result.output, reference);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}
