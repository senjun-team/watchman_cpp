#include <gtest/gtest.h>

#include "core/parser.hpp"
#include "core/service.hpp"

#include <fstream>

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

TEST(Parser, DirectoriesParser) {
    std::string assetName = std::string{TEST_DATA_DIR} + "files_structure.json";
    std::ifstream file(assetName);
    std::stringstream json;
    json << file.rdbuf();

    watchman::Directory rootDirectory = watchman::jsonToDirectory(json.str());
    ASSERT_EQ(rootDirectory.name, "watchman_cpp_dir");
    ASSERT_EQ(rootDirectory.files.size(), 3);
    ASSERT_EQ(rootDirectory.directories.size(), 1);

    auto const & src = rootDirectory.directories.back();
    ASSERT_EQ(src.name, "src");
    ASSERT_EQ(src.files.size(), 2);
    ASSERT_EQ(src.directories.size(), 1);

    auto const & core = src.directories.back();

    ASSERT_EQ(core.name, "core");
    ASSERT_EQ(core.files.size(), 4);
    ASSERT_TRUE(core.directories.empty());
}

TEST(Parser, FileCreator) {
    std::string assetName = std::string{TEST_DATA_DIR} + "files_structure.json";
    std::ifstream file(assetName);
    std::stringstream json;
    json << file.rdbuf();

    watchman::Directory rootDirectory = watchman::jsonToDirectory(json.str());
    makeDirectoryStructure(rootDirectory);

    std::ifstream dirTree(
        "/Users/dmitryshipilov/workspace/Senjun/watchman_cpp/cmake-build-debug/bin/watchman_cpp_dir");
    std::stringstream arch;
    arch << dirTree.rdbuf();
    watchman::makeTar({{arch.str(), "arch"}});
}
