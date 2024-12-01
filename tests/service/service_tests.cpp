#include <gtest/gtest.h>

#include "common.hpp"
#include "common/config.hpp"
#include "core/service.hpp"

#include <thread>

// TODO make containers delete after tests

namespace {

watchman::RunTaskParams getTaskParams(watchman::TaskLauncherType type,
                                      std::vector<std::string> const & cmdLineArgs,
                                      std::string const & sourceCode,
                                      std::string const & testCode) {
    return {{type, cmdLineArgs}, sourceCode, testCode};
}

}  // namespace

TEST(Service, ReadConfig) {
    const watchman::Config cfg = watchman::readConfig(kParams.config);

    ASSERT_TRUE(cfg.threadPoolSize == 10);
    ASSERT_TRUE(cfg.maxContainersAmount == 8);

    auto taskLauncherInfo = cfg.courses.at(watchman::TaskLauncherType::PYTHON_COURSE);
    ASSERT_TRUE(taskLauncherInfo.launched == 1
                && taskLauncherInfo.imageName == "senjun_courses_python");

    taskLauncherInfo = cfg.courses.at(watchman::TaskLauncherType::RUST_COURSE);
    ASSERT_TRUE(taskLauncherInfo.launched == 1
                && taskLauncherInfo.imageName == "senjun_courses_rust");
}

TEST(Service, Run) {
    watchman::Service service(watchman::readConfig(kParams.config));
    auto containerType = watchman::TaskLauncherType::PYTHON_COURSE;
    std::string sourceCode = "print(42)\nprint(42)";
    std::string testingCode = "print(42)";

    watchman::RunTaskParams const params =
        getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_EQ(response.output, "42\r\n42\r\n");
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, TestError) {
    watchman::Service service(watchman::readConfig(kParams.config));
    auto containerType = watchman::TaskLauncherType::PYTHON_COURSE;
    std::string sourceCode = "print(2, 2)\nprint(3, 3)";
    std::string testingCode =
        "from io import StringIO\nimport sys\n\n\nold_stdout = sys.stdout\nsys.stdout = mystdout = StringIO()\n\nprint(2, 2)\nprint(3, 3)\n\nsys.stdout = old_stdout\n\nif 'err_service_unavailable' not in locals():\n    print(\"There is no `err_service_unavailable` variable\")\n    exit(1)\n\nif type(err_service_unavailable) is not int:\n    print(\"Variable is not an integer\")\n    exit(1)\n\nif err_service_unavailable != 503:\n    print(\"Variable value is not 503\")\n    exit(1)";

    watchman::RunTaskParams const params =
        getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);
    auto response = service.runTask(params);
    ASSERT_EQ(response.sourceCode, 2);
    ASSERT_EQ(response.output, "2 2\r\n3 3\r\n");
    ASSERT_EQ(response.testsOutput, "There is no `err_service_unavailable` variable\r\n");
}

TEST(Service, UserSyntaxError) {
    watchman::Service service(watchman::readConfig(kParams.config));
    auto containerType = watchman::TaskLauncherType::PYTHON_COURSE;
    std::string sourceCode = "err_service_unavailable = 503)";
    std::string testingCode =
        "from io import StringIO\nimport sys\n\n\nold_stdout = sys.stdout\nsys.stdout = mystdout = StringIO()\n\nprint(2, 2)\nprint(3, 3)\n\nsys.stdout = old_stdout\n\nif 'err_service_unavailable' not in locals():\n    print(\"There is no `err_service_unavailable` variable\")\n    exit(1)\n\nif type(err_service_unavailable) is not int:\n    print(\"Variable is not an integer\")\n    exit(1)\n\nif err_service_unavailable != 503:\n    print(\"Variable value is not 503\")\n    exit(1)";

    watchman::RunTaskParams const params =
        getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);
    auto response = service.runTask(params);
    ASSERT_EQ(response.sourceCode, 1);
    ASSERT_EQ(
        response.output,
        "  File \"/home/code_runner/task\", line 1\r\n    err_service_unavailable = 503)\r\n                                 ^\r\nSyntaxError: unmatched ')'\r\n");
    ASSERT_FALSE(response.testsOutput.has_value());
}

TEST(Service, UnknownContainerType) {
    watchman::Service service(watchman::readConfig(kParams.config));
    auto containerType = watchman::TaskLauncherType::UNKNOWN;
    std::string sourceCode = "prnt(42)";
    std::string testingCode = "print(42)";
    watchman::RunTaskParams const params =
        getTaskParams(containerType, {}, sourceCode, testingCode);
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == watchman::kInvalidCode);
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, Sleep) {
    watchman::Service service(watchman::readConfig(kParams.config));
    auto containerType = watchman::TaskLauncherType::PYTHON_COURSE;
    std::string sourceCode = "import time\ntime.sleep(2)\nprint(42)";
    std::string testingCode = "print(42)";
    watchman::RunTaskParams const params =
        getTaskParams(containerType, {}, sourceCode, testingCode);

    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, Golang) {
    watchman::Service service(watchman::readConfig(kParams.config));
    auto containerType = watchman::TaskLauncherType::GO_COURSE;
    std::string sourceCode =
        "package main\nimport \"fmt\"\nfunc main() {\tfmt.Println(\"Hello, 世界\")}";
    std::string testingCode =
        "package main\nimport (\"fmt\"\n\"testing\")\nfunc TestMain(m *testing.M) {\tfmt.Println(\"tests are ok\")}";

    watchman::RunTaskParams const params =
        getTaskParams(containerType, {}, sourceCode, testingCode);

    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_TRUE(!response.output.empty());
    ASSERT_TRUE(response.testsOutput.has_value());
}

TEST(Service, RaceCondition) {
    // it is assumed, that python containers less than threads
    watchman::Service service(watchman::readConfig(kParams.config));
    std::thread t1([&service]() {
        auto containerType = watchman::TaskLauncherType::PYTHON_COURSE;
        std::string sourceCode = "import time\ntime.sleep(2)\nprint(42)";
        std::string testingCode = "print(42)";
        watchman::RunTaskParams const params =
            getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);
        auto response = service.runTask(params);
        ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
        ASSERT_EQ(response.output, "42\r\n");
    });

    std::thread t2([&service]() {
        auto containerType = watchman::TaskLauncherType::PYTHON_COURSE;
        std::string sourceCode = "print(69)";
        std::string testingCode = "print(42)";
        watchman::RunTaskParams const params =
            getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);
        auto response = service.runTask(params);
        ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
        ASSERT_EQ(response.output, "69\r\n");
    });

    t1.join();
    t2.join();
}

TEST(Service, AnswerTypes) {
    watchman::Service service(watchman::readConfig(kParams.config));

    // code and tests are ok
    auto containerType = watchman::TaskLauncherType::PYTHON_COURSE;
    std::string sourceCode = "print(42)";
    std::string testingCode = "print(69)";
    watchman::RunTaskParams params =
        getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);

    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
    ASSERT_TRUE(response.output == "42\r\n");
    ASSERT_TRUE(response.testsOutput == "69\r\n");

    // syntax error in user's code
    sourceCode = "print(42";
    testingCode = "lalalala";
    params = getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);

    response = service.runTask(params);

    ASSERT_TRUE(response.sourceCode == watchman::kUserCodeError);
    ASSERT_TRUE(!response.testsOutput.has_value());

    // exception in user code
    sourceCode = "raise";
    testingCode = "lalalala";
    params = getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);
    response = service.runTask(params);

    ASSERT_TRUE(response.sourceCode == watchman::kUserCodeError);
    ASSERT_TRUE(!response.testsOutput.has_value());

    // correct code, but test failed
    sourceCode = "print(42)";
    testingCode = "raise";
    params = getTaskParams(containerType, {"-v code"}, sourceCode, testingCode);
    response = service.runTask(params);

    ASSERT_TRUE(response.sourceCode == watchman::kTestsError);
    ASSERT_TRUE(response.output == "42\r\n");
    ASSERT_TRUE(response.testsOutput.has_value());
}
