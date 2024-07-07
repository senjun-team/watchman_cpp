#include <gtest/gtest.h>

#include "common/common.hpp"
#include "common/logging.hpp"
#include "core/docker_wrapper.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>

using namespace watchman;

std::string const kPythonImage = "senjun_courses_python";

// TODO make containers delete after tests

namespace {
std::string makeCourseTar(std::string code, std::string tests) {
    std::vector<CodeFilename> temp{{std::move(code), kFilenameTask},
                                   {std::move(tests), kFilenameTaskTests}};
    return makeTar(std::move(temp)).str();
}
}  // namespace

TEST(DockerWrapper, run_kill_delete) {
    for (size_t index = 0; index < 10; ++index) {
        // DockerRunParams params{.image = kPythonImage, .tty = true, .memoryLimit = 7000000};
        RunContainer params;
        params.image = kPythonImage;
        params.tty = true;
        params.memory = 7000000;
        DockerWrapper dockerWrapper;
        std::string const id = dockerWrapper.run(std::move(params));
        ASSERT_TRUE(!id.empty());
        ASSERT_TRUE(dockerWrapper.killContainer(id));
        ASSERT_TRUE(dockerWrapper.removeContainer(id));
    }
}

TEST(DockerWrapper, exec) {
    DockerWrapper dockerWrapper;
    RunContainer params{.image = kPythonImage, .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));

    std::string const reference = "42";
    std::vector<std::string> const commands{"echo", reference};

    auto result = dockerWrapper.exec({id, commands});
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.message, "42\r\n");
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, list_of_containers) {
    DockerWrapper dockerWrapper;
    RunContainer params{.image = kPythonImage, .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    auto const containers = dockerWrapper.getAllContainers();
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, is_running) {
    DockerWrapper dockerWrapper;
    RunContainer params{.image = kPythonImage, .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    bool const isRunning = dockerWrapper.isRunning(id);
    ASSERT_TRUE(isRunning);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, get_image) {
    DockerWrapper dockerWrapper;
    RunContainer params{.image = kPythonImage, .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const image = dockerWrapper.getImage(id);
    ASSERT_EQ(image, kPythonImage);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, put_archive) {
    DockerWrapper dockerWrapper;
    RunContainer params{.image = kPythonImage, .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const pathInContainer = "/home/code_runner";

    bool const success =
        dockerWrapper.putArchive({id, pathInContainer, makeCourseTar("print(42)", "print(42)")});
    ASSERT_TRUE(success);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(DockerWrapper, run_user_code) {
    DockerWrapper dockerWrapper;
    RunContainer params{.image = kPythonImage, .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const pathInContainer = "/home/code_runner";

    bool const success =
        dockerWrapper.putArchive({id, pathInContainer, makeCourseTar("print(42)", "print(42)")});
    ASSERT_TRUE(success);

    std::string const reference =
        "42\r\nuser_code_ok_f936a25e\r\n42\r\nuser_solution_ok_f936a25e\r\n";
    std::vector<std::string> const commands{"sh", "run.sh", fmt::format("-f {}", kFilenameTask),
                                            fmt::format("-v code")};

    auto result = dockerWrapper.exec({id, commands});
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.message, reference);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}

TEST(Logger, writeToLog) { Log::info("Hello, logger"); }

TEST(Tar, makeTar) {
    std::string const archiveName = "python_archive.tar";
    std::string const sourceCode = "print(42)\n";
    makeTar({{sourceCode, archiveName}});

    std::remove(archiveName.c_str());
    ASSERT_TRUE(!std::ifstream(archiveName));
}

TEST(DockerWrapper, execute_task) {
    DockerWrapper dockerWrapper;
    RunContainer params{.image = kPythonImage, .tty = true, .memory = 7000000};
    std::string const id = dockerWrapper.run(std::move(params));
    std::string const pathInContainer = "/home/code_runner";
    std::string const sourceCode = "print(42)\n";

    bool const success =
        dockerWrapper.putArchive({id, pathInContainer, makeCourseTar("print(42)", "print(42)")});
    ASSERT_TRUE(success);

    std::string const reference =
        "42\r\nuser_code_ok_f936a25e\r\n42\r\nuser_solution_ok_f936a25e\r\n";
    std::vector<std::string> const commands{"sh", "run.sh", fmt::format("-f {}", kFilenameTask),
                                            fmt::format("-v code")};

    auto result = dockerWrapper.exec({id, commands});
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.message, reference);
    ASSERT_TRUE(dockerWrapper.killContainer(id));
    ASSERT_TRUE(dockerWrapper.removeContainer(id));
}
