#include <gtest/gtest.h>

#include "core/parser.hpp"
#include "core/service.hpp"

#include <common/tar_to_stream.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {
fs::path getAssetPath(std::string const & assetName) {
    fs::path dir(TEST_DATA_DIR);
    fs::path asset(assetName);
    return dir / asset;
}

std::string const kFilesStructureAssets = "files_structure.json";

}  // namespace

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
    std::ifstream file(getAssetPath(kFilesStructureAssets));
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

TEST(Parser, FillPaths) {
    std::ifstream file(getAssetPath(kFilesStructureAssets));
    std::stringstream json;
    json << file.rdbuf();

    watchman::Directory rootDirectory = watchman::jsonToDirectory(json.str());
    auto paths = getPathsToFiles(rootDirectory);
    ASSERT_EQ(paths.size(), 9);
}

TEST(Parser, DISABLED_TarDir) {
    std::ifstream file(getAssetPath(kFilesStructureAssets));
    std::stringstream json;
    json << file.rdbuf();

    watchman::Directory rootDirectory = watchman::jsonToDirectory(json.str());
    auto pathsContents = getPathsToFiles(rootDirectory);

    std::ofstream stream("my_tarball.tar", std::ios::binary | std::ios::trunc);

    for (auto const & pathContent : pathsContents) {
        tar::tar_to_stream(stream, pathContent.path, pathContent.content.data(),
                           pathContent.content.size());
    }
    tar::tar_to_stream_tail(stream);

    ASSERT_TRUE(true);
}
