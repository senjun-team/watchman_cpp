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
