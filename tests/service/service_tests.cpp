#include <gtest/gtest.h>

#include "common.hpp"
#include "core/service.hpp"

#include <thread>

// TODO make containers delete after tests

TEST(Service, ReadConfig) {
    const watchman::Config cfg = watchman::readConfig(kParams.config);

    ASSERT_TRUE(cfg.threadPoolSize.value() == 10);
    ASSERT_TRUE(cfg.maxContainersAmount == 8);

    watchman::Language const & python = cfg.languages.at("python_check");
    ASSERT_TRUE(python.launched == 1 && python.imageName == "senjun_courses_python");

    watchman::Language const & rust = cfg.languages.at("rust_check");
    ASSERT_TRUE(rust.launched == 1 && rust.imageName == "senjun_courses_rust");
}

TEST(Service, Run) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "python_check";
    std::string sourceCode = "print(42)\nprint(42)";
    std::string testingCode = "print(42)";

    watchman::RunTaskParams const params{{std::move(containerType), std::move(sourceCode), {}},
                                         std::move(testingCode)};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_EQ(response.output, "42\r\n42\r\n");
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, UnknownContainerType) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "pythn";
    std::string sourceCode = "prnt(42)";
    std::string testingCode = "print(42)";
    watchman::RunTaskParams const params{{std::move(containerType), std::move(sourceCode), {}},
                                         std::move(testingCode)};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == watchman::kInvalidCode);
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, Sleep) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "python_check";
    std::string sourceCode = "import time\ntime.sleep(2)\nprint(42)";
    std::string testingCode = "print(42)";
    watchman::RunTaskParams const params{{std::move(containerType), std::move(sourceCode), {}},
                                         std::move(testingCode)};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, Golang) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "golang_check";
    std::string sourceCode =
        "package main\nimport \"fmt\"\nfunc main() {\tfmt.Println(\"Hello, 世界\")}";
    std::string testingCode =
        "package main\nimport (\"fmt\"\n\"testing\")\nfunc TestMain(m *testing.M) {\tfmt.Println(\"tests are ok\")}";

    watchman::RunTaskParams const params{{std::move(containerType), std::move(sourceCode), {}},
                                         std::move(testingCode)};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_TRUE(!response.output.empty());
    ASSERT_TRUE(response.testsOutput.has_value());
}

TEST(Service, RaceCondition) {
    // it is assumed, that python containers less than threads
    watchman::Service service(watchman::readConfig(kParams.config));
    std::thread t1([&service]() {
        std::string containerType = "python_check";
        std::string sourceCode = "import time\ntime.sleep(2)\nprint(42)";
        std::string testingCode = "print(42)";
        watchman::RunTaskParams const params{{std::move(containerType), std::move(sourceCode), {}},
                                             std::move(testingCode)};
        auto response = service.runTask(params);
        ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
        ASSERT_EQ(response.output, "42\r\n");
    });

    std::thread t2([&service]() {
        std::string containerType = "python_check";
        std::string sourceCode = "print(69)";
        std::string testingCode = "print(42)";
        watchman::RunTaskParams const params{{std::move(containerType), std::move(sourceCode), {}},
                                             std::move(testingCode)};
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
    std::string const containerType = "python_check";
    std::string sourceCode = "print(42)";
    std::string testingCode = "print(69)";
    watchman::RunTaskParams params{{containerType, sourceCode, {}}, testingCode};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
    ASSERT_TRUE(response.output == "42\r\n");
    ASSERT_TRUE(response.testsOutput == "69\r\n");

    // syntax error in user's code
    sourceCode = "print(42";
    testingCode = "lalalala";
    params = {{containerType, sourceCode, {}}, testingCode};
    response = service.runTask(params);

    ASSERT_TRUE(response.sourceCode == watchman::kUserCodeError);
    ASSERT_TRUE(!response.testsOutput.has_value());

    // exception in user code
    sourceCode = "raise";
    testingCode = "lalalala";
    params = {{containerType, sourceCode, {}}, testingCode};
    response = service.runTask(params);

    ASSERT_TRUE(response.sourceCode == watchman::kUserCodeError);
    ASSERT_TRUE(!response.testsOutput.has_value());

    // correct code, but test failed
    sourceCode = "print(42)";
    testingCode = "raise";
    params = {{containerType, sourceCode, {}}, testingCode};
    response = service.runTask(params);

    ASSERT_TRUE(response.sourceCode == watchman::kTestsError);
    ASSERT_TRUE(response.output == "42\r\n");
    ASSERT_TRUE(response.testsOutput.has_value());
}
