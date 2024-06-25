#include <gtest/gtest.h>

#include "core/parser.hpp"
#include "core/service.hpp"

TEST(Parser, emptyTestString) {
    watchman::Response response;
    response.output = "lalala";
    response.sourceCode = 0;
    response.testsOutput = std::nullopt;

    ASSERT_NE(makeJsonPlayground(std::move(response)), std::string{""});
}

TEST(Parser, notEmptyTestString) {
    watchman::Response response;
    response.output = "lalala";
    response.sourceCode = 0;
    response.testsOutput = "not null";

    ASSERT_NE(makeJsonCourse(std::move(response)), std::string{""});
}
