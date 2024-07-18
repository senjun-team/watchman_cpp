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

TEST(Parser, FoldersParser) {
    std::string assetName = std::string{TEST_DATA_DIR} + "files_structure.json";
    std::ifstream file(assetName);
    std::stringstream json;
    json << file.rdbuf();

    watchman::Folder rootFolder = watchman::jsonToFolder(json.str());
    ASSERT_EQ(rootFolder.name, "watchman_cpp_dir");
    ASSERT_EQ(rootFolder.files.size(), 3);
    ASSERT_EQ(rootFolder.folders.size(), 1);

    auto const & src = rootFolder.folders.back();
    ASSERT_EQ(src.name, "src");
    ASSERT_EQ(src.files.size(), 2);
    ASSERT_EQ(src.folders.size(), 1);

    auto const & core = src.folders.back();

    ASSERT_EQ(core.name, "core");
    ASSERT_EQ(core.files.size(), 4);
    ASSERT_TRUE(core.folders.empty());
}

TEST(Parser, FileCreator) {
    std::string assetName = std::string{TEST_DATA_DIR} + "files_structure.json";
    std::ifstream file(assetName);
    std::stringstream json;
    json << file.rdbuf();

    watchman::Folder rootFolder = watchman::jsonToFolder(json.str());
    makeFolderStructure(rootFolder);
}
