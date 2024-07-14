#include <gtest/gtest.h>

#include "common.hpp"
#include "core/service.hpp"

TEST(Coursess, C_plus_plus) {
    watchman::Service service(watchman::readConfig(kParams.config));
    std::string containerType = "cpp_check";
    std::string sourceCode =
        "#include <iostream>\nusing namespace std;\n int main(){\n\tcout<<\"Hello, world\";}";
    std::string testingCode =
        "#include <iostream>\nusing namespace std;\n int main(){\n\tcout<<\"Hello, world\";}";

    watchman::RunTaskParams const params{{std::move(containerType), std::move(sourceCode), {}},
                                         std::move(testingCode)};
    watchman::Response response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_EQ(response.output, "Hello, world");
    ASSERT_EQ(response.testsOutput.value(), "Hello, world");
    ASSERT_TRUE(!response.output.empty());
}
