#include <gtest/gtest.h>

#include "common.hpp"
#include "common/config.hpp"
#include "common/run_params.hpp"
#include "core/service.hpp"

constexpr std::string_view kCppModulesAsset = "cpp_modules.cpp";

TEST(Coursess, C_plus_plus) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::TaskLauncherType taskType = watchman::TaskLauncherType::CPP_COURSE;
    std::string sourceCode =
        "#include <iostream>\nusing namespace std;\n int main(){\n\tcout<<\"Hello, world\";}";
    std::string testingCode =
        "#include <iostream>\nusing namespace std;\n int main(){\n\tcout<<\"Hello, world\";}";

    watchman::CourseTaskParams params{{
                                          taskType,
                                          {"-v", "code"},
                                      },
                                      std::move(sourceCode),
                                      std::move(testingCode)};
    watchman::Response response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_EQ(response.output, "Hello, world");
    ASSERT_EQ(response.testsOutput.value(), "Hello, world");
    ASSERT_TRUE(!response.output.empty());
}

TEST(Courses, CppModules) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::TaskLauncherType taskType = watchman::TaskLauncherType::CPP_COURSE;
    std::string sourceCode =
        "#include <iostream>\nusing namespace std;\n int main(){\n\tcout<<\"Hello, world\";}";
    std::string testingCode = getFileContent(kCppModulesAsset.data());
    watchman::CourseTaskParams const params{{
                                                taskType,
                                                {"-v", "code"},
                                            },
                                            std::move(sourceCode),
                                            std::move(testingCode)};
    watchman::Response response = service.runTask(params);
    ASSERT_EQ(response.sourceCode, 0);
    ASSERT_EQ(response.output, "Hello, world");
    ASSERT_TRUE(response.testsOutput.has_value());
    ASSERT_NE(response.testsOutput.value().find("all tests passed"), std::string::npos);
}

TEST(Courses, Rust) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::TaskLauncherType taskType = watchman::TaskLauncherType::RUST_COURSE;
    std::string sourceCode =
        "/* You can edit and run this code. */\n\nfn main() {\n    println!(\"Hello world!\");\n}\n";
    std::string testingCode =
        "/* You can edit and run this code. */\n\nfn main() {\n    println!(\"Hello world!\");\n}\n";
    watchman::CourseTaskParams const params{{
                                                taskType,
                                                {"-v", "code"},
                                            },
                                            std::move(sourceCode),
                                            std::move(testingCode)};
    watchman::Response response = service.runTask(params);
    ASSERT_EQ(response.sourceCode, 0);
    ASSERT_EQ(response.output, "Hello world!\r\n");
    ASSERT_EQ(response.testsOutput.value(), "Hello world!\r\n");
}

TEST(Courses, DISABLED_Haskell) {
    watchman::Service service(watchman::readConfig(kParams.config));
    watchman::TaskLauncherType taskType = watchman::TaskLauncherType::HASKELL_COURSE;
    std::string sourceCode =
        "{- You can edit and run this code. -}\n\nmodule Main where\n\nmain :: IO ()\nmain = putStrLn (\"Hello world!\")\n";
    std::string testingCode = "print(\"Hello world!\")";  // test written in python
    watchman::CourseTaskParams const params{{
                                                taskType,
                                                {"-v", "code"},
                                            },
                                            std::move(sourceCode),
                                            std::move(testingCode)};
    watchman::Response response = service.runTask(params);
    ASSERT_EQ(response.sourceCode, 0);
    ASSERT_EQ(response.output, "Hello world!\r\n");
    ASSERT_EQ(response.testsOutput.value(), "Hello world!\r\n");
}