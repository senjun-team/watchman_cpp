#include <gtest/gtest.h>

#include "common/common.hpp"
#include "common/logging.hpp"
#include "core/docker_wrapper.hpp"

#include <cstdio>
#include <fstream>

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

TEST(DockerWrapper, list_of_containers) {
    DockerWrapper dockerWrapper;
    DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    auto const containers = dockerWrapper.getAllContainers();
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, is_running) {
    DockerWrapper dockerWrapper;
    DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    bool const isRunning = dockerWrapper.isRunning(id);
    ASSERT_TRUE(isRunning);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, get_image) {
    DockerWrapper dockerWrapper;
    DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const image = dockerWrapper.getImage(id);
    ASSERT_EQ(image, kPythonImage);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, put_archive) {
    DockerWrapper dockerWrapper;
    DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const pathInContainer = "/home/code_runner";
    std::string pythonTar = std::string{TEST_DATA_DIR} + "example.tgz";

    bool const success = dockerWrapper.putArchive({id, pathInContainer, pythonTar});
    ASSERT_TRUE(success);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, run_user_code) {
    DockerWrapper dockerWrapper;
    DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const pathInContainer = "/home/code_runner";
    std::string pythonTar = std::string{TEST_DATA_DIR} + "example.tgz";

    bool const success = dockerWrapper.putArchive({id, pathInContainer, pythonTar});
    ASSERT_TRUE(success);

    std::string const reference = "42";
    std::vector<std::string> const args{"sh", "run.sh", "example.py"};

    auto result = dockerWrapper.exec({args, id});
    ASSERT_TRUE(result.exitCode == 0);
    ASSERT_EQ(result.output, reference);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(Logger, writeToLog) { Log::info("Hello, logger"); }

TEST(Tar, makeTar) {
    std::string const archiveName = "python_archive.tar";
    std::string const sourceCode = "print(42)\n";
    makeTar(archiveName, sourceCode);

    std::remove(archiveName.c_str());
    ASSERT_TRUE(!std::ifstream(archiveName));
}
