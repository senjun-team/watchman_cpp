#include <gtest/gtest.h>

#include "core/service.hpp"

TEST(Service, Run) {
    watchman::Service service;
    watchman::RunTaskParams const params{"python", "prnt(42)", "print(42)"};
    auto response = service.runTask(params);
    ASSERT_TRUE(response.sourceCode == 0);
    ASSERT_TRUE(!response.output.empty());
}
