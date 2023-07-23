#include <gtest/gtest.h>

#include "core/service.hpp"

#include <thread>

struct ServiceParams {
    std::string const config = std::string{TEST_DATA_DIR} + "config.json";
};

static ServiceParams const kParams;

TEST(Service, ReadConfig) {
    const watchman::Config cfg = watchman::readConfig(kParams.config);

    ASSERT_TRUE(cfg.threadPoolSize.value() == 10);
    ASSERT_TRUE(cfg.maxContainersAmount == 8);

    watchman::Language const & python = cfg.languages.at(watchman::ContainerType::Python);
    ASSERT_TRUE(python.launched == 1 && python.imageName == "senjun_courses_python");

    watchman::Language const & rust = cfg.languages.at(watchman::ContainerType::Rust);
    ASSERT_TRUE(rust.launched == 1 && rust.imageName == "senjun_courses_rust");
}

TEST(Service, Run) {
    watchman::Service service(watchman::readConfig(kParams.config));
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
    watchman::Service service(watchman::readConfig(kParams.config));
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
    watchman::Service service(watchman::readConfig(kParams.config));
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
    watchman::Service service(watchman::readConfig(kParams.config));
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
