#include <gtest/gtest.h>

#include "core/service.hpp"

#include <thread>

TEST(Service, Run) {
    watchman::Service service;
    std::string containerType = "python";
    std::string sourceCode = "print(42)";
    std::string testingCode = "print(42)";

    watchman::RunTaskParams const params{std::move(containerType), std::move(sourceCode),
                                         std::move(testingCode)};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, UnknownContainerType) {
    watchman::Service service;
    std::string containerType = "pythn";
    std::string sourceCode = "prnt(42)";
    std::string testingCode = "print(42)";
    watchman::RunTaskParams const params{std::move(containerType), std::move(sourceCode),
                                         std::move(testingCode)};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == watchman::kInvalidCode);
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, Sleep) {
    watchman::Service service;
    std::string containerType = "python";
    std::string sourceCode = "import time\ntime.sleep(2)\nprint(42)";
    std::string testingCode = "print(42)";
    watchman::RunTaskParams const params{std::move(containerType), std::move(sourceCode),
                                         std::move(testingCode)};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
    ASSERT_TRUE(!response.output.empty());
}

TEST(Service, RaceCondition) {
    // it is assumed, that python containers less than threads
    watchman::Service service;
    std::thread t1([&service]() {
        std::string containerType = "python";
        std::string sourceCode = "import time\ntime.sleep(2)\nprint(42)";
        std::string testingCode = "print(42)";
        watchman::RunTaskParams const params{std::move(containerType), std::move(sourceCode),
                                             std::move(testingCode)};
        auto response = service.runTask(params);
        ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
        ASSERT_EQ(response.output, "42");
    });

    std::thread t2([&service]() {
        std::string containerType = "python";
        std::string sourceCode = "print(69)";
        std::string testingCode = "print(42)";
        watchman::RunTaskParams const params{std::move(containerType), std::move(sourceCode),
                                             std::move(testingCode)};
        auto response = service.runTask(params);
        ASSERT_TRUE(response.sourceCode == watchman::kSuccessCode);
        ASSERT_EQ(response.output, "69");
    });

    t1.join();
    t2.join();
}
